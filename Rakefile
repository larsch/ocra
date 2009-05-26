# -*- ruby -*-

require 'rubygems'
require 'hoe'
require 'bin/ocra'

Hoe.new('ocra', Ocra::VERSION) do |p|
  p.developer('Lars Christensen', 'larsch@belunktum.dk')
end

task :stub do
  sh "mingw32-make -C src"
  cp 'src/stub.exe', 'share/ocra/stub.exe'
  cp 'src/stubw.exe', 'share/ocra/stubw.exe'
  cp 'src/edicon.exe', 'share/ocra/edicon.exe'
end

task :test => :stub

task :standalone => [ 'bin/ocrasa.rb' ]

standalone_zip = "bin/ocrasa-#{ENV['VERSION']}.zip"

file standalone_zip => 'bin/ocrasa.rb' do
  chdir('bin') do
    system("zip ocrasa-#{ENV['VERSION']}.zip ocrasa.rb")
  end
end

task :release_standalone => standalone_zip do
  sh "rubyforge add_release ocra ocra-standalone #{Ocra::VERSION} #{standalone_zip}"
end

file 'bin/ocrasa.rb' => [ 'bin/ocra.rb', 'share/ocra/stub.exe', 'share/ocra/stubw.exe', 'share/ocra/lzma.exe', 'share/ocra/edicon.exe' ] do
  cp 'bin/ocra.rb', 'bin/ocrasa.rb'
  File.open("bin/ocrasa.rb", "a") do |f|
    f.puts "__END__"
    
    stub = File.open("share/ocra/stub.exe", "rb") {|g| g.read}
    stub64 = [stub].pack("m")
    f.puts stub64.size
    f.puts stub64

    stub = File.open("share/ocra/stubw.exe", "rb") {|g| g.read}
    stub64 = [stub].pack("m")
    f.puts stub64.size
    f.puts stub64
    
    lzma = File.open("share/ocra/lzma.exe", "rb") {|g| g.read}
    lzma64 = [lzma].pack("m")
    f.puts lzma64.size
    f.puts lzma64

    lzma = File.open("share/ocra/edicon.exe", "rb") {|g| g.read}
    lzma64 = [lzma].pack("m")
    f.puts lzma64.size
    f.puts lzma64
  end
end

task :clean do
  rm_rf Dir.glob("bin/*.exe")
  sh "mingw32-make -C src clean"
end

task :test_standalone => :standalone do
  ENV['TESTED_OCRA'] = 'ocrasa.rb'
  system("rake test")
  ENV['TESTED_OCRA'] = nil
end

# vim: syntax=Ruby
