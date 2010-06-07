require "test/unit"
require "tmpdir"
require "fileutils"
require "rbconfig"

require 'rbconfig'
puts "Version: #{RUBY_VERSION}-p#{RUBY_PATCHLEVEL}"
puts "Architecture: #{RbConfig::CONFIG['arch']}"
puts "Host: #{RbConfig::CONFIG['host']}"

begin
  require "rubygems"
  gem 'win32-api', '>=1.2.0'
  require "win32/api"
  $have_win32_api = true
rescue LoadError => e
  $have_win32_api = false
end

include FileUtils

class TestOcra < Test::Unit::TestCase

  # Default arguments for invoking OCRA when running tests.
  DefaultArgs = [ '--quiet', '--no-lzma' ]
  DefaultArgs.push '--no-autodll' if not $have_win32_api

  # Name of the tested ocra script.
  TESTED_OCRA = ENV['TESTED_OCRA'] || 'ocra'

  # Root of OCRA.
  OcraRoot = File.expand_path(File.join(File.dirname(__FILE__), '..'))

  # Path to test fixtures.
  FixturePath = File.expand_path(File.join(File.dirname(__FILE__), 'fixtures'))

  attr_reader :ocra

  def initialize(*args)
    super(*args)
    @testnum = 0
    @ocra = File.expand_path(File.join(File.dirname(__FILE__), '..', 'bin', TESTED_OCRA))
    ENV['RUBYOPT'] = ""
  end

  # Sets up an directory with a copy of a fixture and yields to the
  # block, then cleans up everything. A fixture here is a hierachy of
  # files located in test/fixtures.
  def with_fixture(name)
    path = File.join(FixturePath, name)
    cp_r path, '.'
    begin
      cd name do
        yield
      end
    ensure
      rm_rf name
    end
  end

  # Sets up temporary environment variables and yields to the
  # block. When the block exits, the environment variables are set
  # back to their original values.
  def with_env(hash)
    old = {}
    hash.each do |k,v|
      old[k] = ENV[k]
      ENV[k] = v
    end
    begin
      yield
    ensure
      hash.each do |k,v|
        ENV[k] = old[k]
      end
    end
  end

  def with_tmpdir
    tempdirname = File.join(ENV['TEMP'], ".ocratest-#{$$}-#{rand 2**32}").tr('\\','/')
    Dir.mkdir tempdirname
    begin
      FileUtils.cd tempdirname do
        yield
      end
    ensure
      FileUtils.rm_rf tempdirname
    end
  end

  def with_exe(name)
    orig_exe = File.expand_path(name)
    with_tmpdir do
      cp orig_exe, File.join('.', File.basename(name))
      yield
    end
  end
  
  # Test setup method. Creates a tempory directory to work in and
  # changes to it. 
  def setup
    @testnum += 1
    @tempdirname = ".ocratest-#{$$}-#{@testnum}"
    Dir.mkdir @tempdirname
    Dir.chdir @tempdirname
  end

  # Test cleanup method. Exits the temporary directory and deletes it.
  def teardown
    Dir.chdir '..'
    FileUtils.rm_rf @tempdirname
  end

  # Hello world test. Test that we can build and run executables.
  def test_helloworld
    with_fixture 'helloworld' do
      assert system("ruby", ocra, "helloworld.rb", *DefaultArgs)
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  # Test that executables can writing a file to the current working
  # directory.
  def test_writefile
    with_fixture 'writefile' do
      assert system("ruby", ocra, "writefile.rb", *DefaultArgs)
      assert File.exist?("writefile.exe")
      assert system("writefile.exe")
      assert File.exist?("output.txt")
      assert "output", File.read("output.txt")
    end
  end

  # Test that scripts can exit with a specific exit status code.
  def test_exitstatus
    with_fixture 'exitstatus' do
      assert system("ruby", ocra, "exitstatus.rb", *DefaultArgs)
      system("exitstatus.exe")
      assert_equal 167, $?.exitstatus
    end
  end

  # Test that arguments are passed correctly to scripts.
  def test_arguments
    with_fixture 'arguments' do
      assert system("ruby", ocra, "arguments.rb", *DefaultArgs)
      assert File.exist?("arguments.exe")
      system("arguments.exe foo \"bar baz\"")
      assert_equal 5, $?.exitstatus
    end
  end

  # Test that the standard output from a script can be redirected to a
  # file.
  def test_stdout_redir
    with_fixture 'stdoutredir' do
      assert system("ruby", ocra, "stdoutredir.rb", *DefaultArgs)
      assert File.exist?("stdoutredir.exe")
      system("stdoutredir.exe > output.txt")
      assert File.exist?("output.txt")
      assert_equal "Hello, World!\n", File.read("output.txt")
    end
  end

  # Test that the standard input to a script can be redirected from a
  # file.
  def test_stdin_redir
    with_fixture 'stdinredir' do
      assert system("ruby", ocra, "stdinredir.rb", *DefaultArgs)
      assert File.exist?("stdinredir.exe")
      system("stdinredir.exe < input.txt")
      assert 104, $?.exitstatus
    end
  end

  # Test that executables can include dll's using the --dll
  # option. Sets PATH=. while running the executable so that it can't
  # find the DLL from the Ruby installation.
  def test_gdbmdll
    args = DefaultArgs.dup
    if not $have_win32_api
      gdbmdll = Dir.glob(File.join(RbConfig::CONFIG['bindir'], 'gdbm*.dll'))[0]
      return if gdbmdll.nil?
      args.push '--dll', File.basename(gdbmdll)
    end
    
    with_fixture 'gdbmdll' do
      assert system("ruby", ocra, "gdbmdll.rb", *args)
      with_env 'PATH' => '.' do
        system("gdbmdll.exe")
        assert_equal 104, $?.exitstatus
      end
    end
  end

  # Test that scripts can require a file relative to the location of
  # the script and that such files are correctly added to the
  # executable.
  def test_relative_require
    with_fixture 'relativerequire' do
      assert system("ruby", ocra, "relativerequire.rb", *DefaultArgs)
      assert File.exist?("relativerequire.exe")
      system("relativerequire.exe")
      assert_equal 160, $?.exitstatus
    end
  end

  # Test that autoloaded files which are not actually loaded while
  # running the script through Ocra are included in the resulting
  # executable.
  def test_autoload
    with_fixture 'autoload' do
      assert system("ruby", ocra, "autoload.rb", *DefaultArgs)
      assert File.exist?("autoload.exe")
      File.unlink('foo.rb')
      assert system("autoload.exe")
    end
  end

  # Test that autoload statement which point to non-existing files are
  # ignored by Ocra (a warning may be logged).
  def test_autoload_missing
    with_fixture 'autoloadmissing' do
      args = DefaultArgs.dup
      args.push '--no-warnings'
      assert system("ruby", ocra, "autoloadmissing.rb", *args)
      assert File.exist?("autoloadmissing.exe")
      assert system("autoloadmissing.exe")
    end
  end

  # Test that Ocra picks up autoload statement nested in modules.
  def test_autoload_nested
    with_fixture 'autoloadnested' do
      assert system("ruby", ocra, "autoloadnested.rb", *DefaultArgs)
      assert File.exist?("autoloadnested.exe")
      File.unlink('foo.rb')
      assert system("autoloadnested.exe")
    end
  end

  # Test that we can use custom include paths when invoking Ocra (ruby
  # -I somepath). In this case the lib scripts are put in the src/
  # directory.
  def test_relative_loadpath1_ilib
    with_fixture 'relloadpath1' do
      assert system('ruby', '-I', 'lib', ocra, 'relloadpath1.rb', *DefaultArgs)
      assert File.exist?('relloadpath1.exe')
      assert system('relloadpath1.exe')
    end
  end

  # Same as above with './lib'
  def test_relative_loadpath_idotlib
    with_fixture 'relloadpath1' do
      assert system('ruby', '-I', './lib', ocra, 'relloadpath1.rb', *DefaultArgs)
      assert File.exist?('relloadpath1.exe')
      assert system('relloadpath1.exe')
    end
  end

  # Test that we can use custom include paths when invoking Ocra (env
  # RUBYLIB=lib). In this case the lib scripts are put in the src/
  # directory.
  def test_relative_loadpath_rubyliblib
    with_fixture 'relloadpath1' do
      with_env 'RUBYLIB' => 'lib' do
        assert system('ruby', ocra, 'relloadpath1.rb', *DefaultArgs)
        assert File.exist?('relloadpath1.exe')
        assert system('relloadpath1.exe')
      end
    end
  end

  # Same as above with './lib'
  def test_relative_loadpath_rubylibdotlib
    with_fixture 'relloadpath1' do
      with_env 'RUBYLIB' => './lib' do
        assert system('ruby', ocra, 'relloadpath1.rb', *DefaultArgs)
        assert File.exist?('relloadpath1.exe')
        assert system('relloadpath1.exe')
      end
    end
  end

  # Relative path with .. prefix (../lib).
  def test_relative_loadpath2_idotdotlib
    with_fixture 'relloadpath2' do
      cd 'src' do
        assert system('ruby', '-I', '../lib', ocra, 'relloadpath2.rb', *DefaultArgs)
        assert File.exist?('relloadpath2.exe')
        assert system('relloadpath2.exe')
      end
    end
  end

  # Test that scripts which modify $LOAD_PATH with a relative path
  # (./lib) work correctly.
  def test_relloadpath3
    with_fixture 'relloadpath3' do
      assert system('ruby', ocra, 'relloadpath3.rb', *DefaultArgs)
      assert File.exist?('relloadpath3.exe')
      assert system('relloadpath3.exe')
    end
  end

  # Test that scripts which modify $LOAD_PATH with a relative path
  # (../lib) work correctly.
  def test_relloadpath4
    with_fixture 'relloadpath4' do
      cd 'src' do
        assert system('ruby', ocra, 'relloadpath4.rb', *DefaultArgs)
        assert File.exist?('relloadpath4.exe')
        assert system('relloadpath4.exe')
      end
    end
  end

  # Test that ocra.rb accepts --version and outputs the version number.
  def test_version
    assert_match(/^Ocra \d+(\.\d)+$/, `ruby \"#{ocra}\" --version`)
  end

  # Test that ocra.rb accepts --icon.
  def test_icon
    with_fixture 'helloworld' do
      icofile = File.join(OcraRoot, 'src', 'vit-ruby.ico')
      assert system("ruby", ocra, '--icon', icofile, "helloworld.rb", *DefaultArgs)
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  # Test that additional non-script files can be added to the
  # executable and used by the script.
  def test_resource
    with_fixture 'resource' do
      assert system("ruby", ocra, "resource.rb", "resource.txt", "res/resource.txt", *DefaultArgs)
      assert File.exist?("resource.exe")
      assert system("resource.exe")
    end
  end

  # Test that when exceptions are thrown, no executable will be built.
  def test_exception
    with_fixture 'exception' do
      system("ruby \"#{ocra}\" exception.rb #{DefaultArgs.join(' ')} 2>NUL")
      assert $?.exitstatus != 0
      assert !File.exist?("exception.exe")
    end
  end

  # Test that the RUBYOPT environment variable is preserved.
  def test_rubyopt
    with_fixture 'environment' do
      with_env "RUBYOPT" => "-rtime" do
        assert system("ruby", ocra, "environment.rb", *DefaultArgs)
        assert system("environment.exe")
        env = Marshal.load(File.open("environment", "rb") { |f| f.read })
        assert_equal "-rtime", env['RUBYOPT']
      end
    end
  end

  def test_exit
    with_fixture 'exit' do
      assert system("ruby", ocra, "exit.rb", *DefaultArgs)
      assert File.exist?("exit.exe")
      assert system("exit.exe")
    end
  end

  def test_ocra_executable_env
    with_fixture 'environment' do
      assert system("ruby", ocra, "environment.rb", *DefaultArgs)
      assert system("environment.exe")
      env = Marshal.load(File.open("environment", "rb") { |f| f.read })
      expected_path = File.expand_path("environment.exe").tr('/','\\')
      assert_equal expected_path, env['OCRA_EXECUTABLE']
    end
  end

  def test_hierarchy
    with_fixture 'hierarchy' do
      assert system("ruby", ocra, "hierarchy.rb", "assets/**/*", *DefaultArgs)
      with_exe "hierarchy.exe" do
        assert system("hierarchy.exe")
      end
    end
  end

  def test_temp_with_space
    with_fixture 'helloworld' do
      assert system("ruby", ocra, "helloworld.rb", *DefaultArgs)
      tempdir = File.expand_path("temporary directory")
      mkdir_p tempdir
      with_exe "helloworld.exe" do
        with_env "TMP" => tempdir.tr('/','\\') do
          assert system("helloworld.exe")
        end
      end
    end
  end

  def test_abspath
    with_fixture "helloworld" do
      assert system("ruby", ocra, File.expand_path("helloworld.rb"), *DefaultArgs)
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  def test_abspath_outside
    with_fixture "helloworld" do
      mkdir "build"
      cd "build" do
        assert system("ruby", ocra, File.expand_path("../helloworld.rb"), *DefaultArgs)
      end
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  def test_relpath
    with_fixture "helloworld" do
      assert system("ruby", ocra, "./helloworld.rb", *DefaultArgs)
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  def test_relpath_outside
    with_fixture "helloworld" do
      mkdir "build"
      cd "build" do
        assert system("ruby", ocra, "../helloworld.rb", *DefaultArgs)
      end
      assert File.exist?("helloworld.exe")
      assert system("helloworld.exe")
    end
  end

  
end
