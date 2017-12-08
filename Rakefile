# -*- ruby -*-

require 'rubygems'
require 'hoe'

Hoe.plugin :minitest

spec = Hoe.spec 'ocra' do
  developer "Lars Christensen", "larsch@belunktum.dk"
  license "MIT"
end

spec.urls.each { |url| url.chomp! }

desc "task for fast iteration"
task :do_it do
  sh "gem uninstall ocra"
  sh "rake install_gem" rescue nil
  puts "installing the bitch"
  sh "gem install pkg/ocra-1.4.666.gem"
  sh "rm hello.exe"  rescue nil
  sh "ocra hello.rb"
  sh "cp hello.exe hello-signed.exe"
  puts "Now creating a signed copy in hello-signed.exe"
  sh "signtool sign /f ./CARoot.pfx /p Test123 hello-signed.exe"
  puts "output of hello.exe"
  sh "./hello.exe"
  puts "output of hello-signed.exe"
  sh "./hello-signed.exe" rescue nil
end

task :build_stub do
  sh "mingw32-make -C src"
  cp 'src/stub.exe', 'share/ocra/stub.exe'
  cp 'src/stubw.exe', 'share/ocra/stubw.exe'
  cp 'src/edicon.exe', 'share/ocra/edicon.exe'
end

file 'share/ocra/stub.exe' => :build_stub
file 'share/ocra/stubw.exe' => :build_stub
file 'share/ocra/edicon.exe' => :build_stub

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
  rm_f Dir["{bin,samples}/*.exe"]
  rm_f Dir["share/ocra/{stub,stubw,edicon}.exe"]
  sh "mingw32-make -C src clean"
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
