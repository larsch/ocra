#!/usr/bin/env ruby
# -*- ruby -*-

module Ocra
  Signature = [0x41, 0xb6, 0xba, 0x4e]
  OP_END = 0
  OP_CREATE_DIRECTORY = 1
  OP_CREATE_FILE = 2
  OP_CREATE_PROCESS = 3
  OP_DECOMPRESS_LZMA = 4
  OP_SETENV = 5
  
  def Ocra.find_images
    if defined?(DATA)
      $sebimage = DATA.read(DATA.readline.to_i).unpack("m")[0]
      $lzmaimage = DATA.read(DATA.readline.to_i).unpack("m")[0]
      $lzmapath = File.join(ENV['TEMP'], 'lzma.exe').tr('/','\\')
      puts $lzmapath
      puts $lzmaimage.size
      File.open($lzmapath, "wb") { |f| f << $lzmaimage }
    else
      $sebimage = File.open(File.join(File.dirname(__FILE__), '../share/ocra/stub.exe'), "rb") { |f| f.read }
      $lzmapath = File.join(File.dirname(__FILE__), '../share/ocra/lzma.exe')
      raise "lzma.exe not found" unless File.exist?($lzmapath)
    end
  end
  
  def Ocra.build_exe
    libs = []

    if $autoload
      # Force loading autoloaded
      modules_checked = []
      loop do
        modules_to_check = []
        ObjectSpace.each_object(Module) do |m|
          modules_to_check << m unless modules_checked.include?(m)
        end
        break if modules_to_check.empty?
        modules_to_check.each do |m|
          modules_checked << m
          m.constants.each do |c|
            if m.autoload?(c)
              begin
                m.const_get(c)
              rescue LoadError
                puts "=== WARNING: #{m}::#{c} was not loadable"
              end
            end
          end
        end
      end
    end

    features = $LOADED_FEATURES.dup

    require 'rbconfig'
    exec_prefix = RbConfig::CONFIG['exec_prefix']
    src_prefix = File.expand_path(File.dirname($files[0]))
    sitelibdir = RbConfig::CONFIG['sitelibdir']
    instsitelibdir = sitelibdir[exec_prefix.size+1..-1]

    # Find loaded files
    features.each do |filename|
      path = $:.find { |p| File.exist?(File.expand_path(filename, p)) }
      if path
        fullpath = File.expand_path(filename, path)
        if fullpath.index(exec_prefix) == 0
          libs << [ fullpath, fullpath[exec_prefix.size+1..-1] ]
        elsif fullpath.index(src_prefix) == 0
          libs << [ fullpath, "src/" + fullpath[src_prefix.size+1..-1]]
        else
          libs << [ fullpath, File.join(instsitelibdir, filename) ]
        end
      else
        puts "=== WARNING: Couldn't find #{filename}"
      end
    end

    # Find gemspecs to include
    if defined?(Gem)
      gemspecs = Gem.loaded_specs.map { |name,info| info.loaded_from }
    else
      gemspecs = []
    end

    find_images
    require 'rbconfig'
    bindir = RbConfig::CONFIG['bindir']
    libruby_so = RbConfig::CONFIG['LIBRUBY_SO']

    executable = $files[0].sub(/(\.rbw?)?$/, '.exe')

    puts "=== Building #{executable}" unless $quiet
    SebBuilder.new(executable) do |sb|
      sb.mkdir('src')

      $files.each do |file|
        path = File.join('src', file).tr('/','\\')
        sb.createfile(file, path)
      end
      sb.mkdir('bin')
      
      if ($files[0] =~ /\.rbw$/ && !$force_windows) || $force_console
        rubyexe = "ruby.exe"
      else
        rubyexe = "ruby.exe"
      end
      
      sb.createfile(File.join(bindir, rubyexe), "bin\\" + rubyexe)
      
      sb.createfile(File.join(bindir, libruby_so), "bin\\#{libruby_so}")
      $extra_dlls.each { |dll|
        sb.createfile(File.join(bindir, dll), File.join("bin", dll).tr('/','\\'))
      }
      #sb.createfile('c:\lang\Ruby-186-27\lib\ruby\gems\1.8\specifications\wxruby-2.0.0-x86-mswin32-60.gemspec',
      #              'lib\ruby\gems\1.8\specifications\wxruby-2.0.0-x86-mswin32-60.gemspec')

      exec_prefix = RbConfig::CONFIG['exec_prefix']
      gemspecs.each { |gemspec|
        pref = gemspec[0,exec_prefix.size]
        path = gemspec[exec_prefix.size+1..-1]
        if pref != exec_prefix
          raise "#{gemspec} does not exist in the Ruby installation. Don't know where to put it."
        end
        sb.createfile(gemspec, path.tr('/','\\'))
      }

      libs.each { |path, tgt|
        # p [path,tgt]
        dst = tgt.tr('/', '\\')
        sb.createfile(path, dst)
      }

      sb.setenv('RUBYOPT', '')
      sb.setenv('RUBYLIB', '')
      sb.createprocess("bin\\" + rubyexe, "#{rubyexe} \xff\\src\\" + $files[0])
      puts "=== Compressing" unless $quiet or not $lzma_mode
    end
    puts "=== Finished (Final size was #{File.size(executable)})" unless $quiet
  end
  
  class SebBuilder
    def initialize(path)
      @paths = {}
      File.open(path, "wb") do |f|
        f.write($sebimage)
        if $lzma_mode
          @of = ""
        else
          @of = f
        end
        yield(self)

        if $lzma_mode
          File.open("tmpin", "wb") { |tmp| tmp.write(@of) }
          system("#{$lzmapath} e tmpin tmpout 2>NUL") or fail
          @c = File.open("tmpout", "rb") { |tmp| tmp.read }
          f.write([OP_DECOMPRESS_LZMA, @c.size, @c].pack("VVA*"))
          f.write([OP_END].pack("V"))
        else
          f.write(@of) if $lzma_mode
        end

        f.write([OP_END].pack("V"))
        f.write([$sebimage.size].pack("V"))
        f.write(Signature.pack("C*"))
      end
    end
    def mkdir(path)
      @paths[path] = true
      puts "m #{path}" unless $quiet
      @of << [OP_CREATE_DIRECTORY, path].pack("VZ*")
    end
    def ensuremkdir(tgt)
      return if tgt == "."
      if not @paths[tgt]
        ensuremkdir(File.dirname(tgt))
        mkdir(tgt)
      end
    end
    def createfile(src, tgt)
      ensuremkdir(File.dirname(tgt))
      str = File.open(src, "rb") { |s| s.read }
      puts "a #{tgt}" unless $quiet
      @of << [OP_CREATE_FILE, tgt, str.size, str].pack("VZ*VA*")
    end
    def createprocess(image, cmdline)
      puts "l #{image} #{cmdline}" unless $quiet
      @of << [OP_CREATE_PROCESS, image, cmdline].pack("VZ*Z*")
    end
    def setenv(name, value)
      puts "e #{name} #{value}" unless $quiet
      @of << [OP_SETENV, name, value].pack("VZ*Z*")
    end
    def close
      @of.close
    end
  end # class SebBuilder
  
end # module Ocra

$lzma_mode = true
$extra_dlls = []
$files = []
$autoload = true

usage = <<EOF
ocra [--dll dllname] [--no-lzma] script.rb

--dll dllname    Include additional DLLs from the Ruby bindir.
--no-lzma        Disable LZMA compression of the executable.
--quiet          Suppress output.
--help           Display this information.
--windows        Force Windows application (rubyw.exe)
--console        Force console application (ruby.exe)
--no-autoload    Don't load/include script.rb's autoloads
EOF

while arg = ARGV.shift
  case arg
  when /\A--(no-)?lzma\z/
    $lzma_mode = !$1
  when /\A--dll\z/
    $extra_dlls << ARGV.shift
  when /\A--quiet\z/
    $quiet = true
  when /\A--windows\z/
    $force_windows = true
  when /\A--console\z/
    $force_console = true
  when /\A--no-autoload\z/
    $autoload = false
  when /\A--help\z/, /\A--/
    puts usage
    exit
  else
    $files << arg
  end
end

if $files.empty?
  puts usage
  exit
end

if defined? Gem
  puts "=== Warning: Rubygems is loaded. Rubygems will be included the archive."
  puts "        RUBYOPT=#{ENV['RUBYOPT']}"
end

at_exit do
  Ocra.build_exe
  exit
end

puts "=== Loading script to check dependencies" unless $quiet
load $files[0]

