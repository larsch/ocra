# -*- ruby -*-

require 'rubygems'
require 'hoe'

Hoe.spec 'ocra' do
  developer "Lars Christensen", "larsch@belunktum.dk"
  self.readme_file = 'README.rdoc'
end

task :build_stub do
  sh "mingw32-make -C src"
  cp 'src/stub.exe', 'share/ocra/stub.exe'
  cp 'src/stubw.exe', 'share/ocra/stubw.exe'
  cp 'src/edicon.exe', 'share/ocra/edicon.exe'
end

file 'share/ocra/stub.exe' => :build_stub

file 'README.txt' => 'README.rdoc' do
  cp 'README.rdoc', 'README.txt'
end

task :test => :build_stub

task :standalone => [ 'bin/ocrasa.rb' ]

standalone_zip = "bin/ocrasa-#{ENV['VERSION']}.zip"

file standalone_zip => 'bin/ocrasa.rb' do
  chdir 'bin' do
    sh "zip", "ocrasa-#{ENV['VERSION']}.zip", "ocrasa.rb"
  end
end

task :release_standalone => standalone_zip do
  load 'bin/ocra'
  sh "rubyforge add_release ocra ocra-standalone #{Ocra::VERSION} #{standalone_zip}"
end

file 'bin/ocrasa.rb' => [ 'bin/ocra', 'share/ocra/stub.exe', 'share/ocra/stubw.exe', 'share/ocra/lzma.exe', 'share/ocra/edicon.exe' ] do
  cp 'bin/ocra', 'bin/ocrasa.rb'
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
  rm_f 'share/ocra/stub.exe'
  rm_f 'share/ocra/stubw.exe'
  rm_f 'share/ocra/edicon.exe'
end

task :test_standalone => :standalone do
  ENV['TESTED_OCRA'] = 'ocrasa.rb'
  system("rake test")
  ENV['TESTED_OCRA'] = nil
end

task :release_docs => :redocs do
  sh "pscp -r doc/* larsch@ocra.rubyforge.org:/var/www/gforge-projects/ocra"
end


# vim: syntax=Ruby
