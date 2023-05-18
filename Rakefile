# -*- ruby -*-

require "rubygems"
require 'bundler/setup'
require "hoe"

Hoe.plugin :minitest

spec = Hoe.spec "ocran" do
  developer "Lars Christensen", "larsch@belunktum.dk"
  developer "Andi Idogawa", "andi@idogawa.com"
  license "MIT"
end

spec.urls.each { |key, url| url.chomp! }

task :build_stub do
  sh "ridk exec make -C src"
  cp "src/stub.exe", "share/ocran/stub.exe"
  cp "src/stubw.exe", "share/ocran/stubw.exe"
  cp "src/edicon.exe", "share/ocran/edicon.exe"
end

file "share/ocran/stub.exe" => :build_stub
file "share/ocran/stubw.exe" => :build_stub
file "share/ocran/edicon.exe" => :build_stub

task :test => :build_stub

task :standalone => ["exe/ocrasa.rb"]

standalone_zip = "exe/ocrasa-#{ENV["VERSION"]}.zip"

file standalone_zip => "exe/ocrasa.rb" do
  chdir "exe" do
    sh "zip", "ocransa-#{ENV["VERSION"]}.zip", "ocransa.rb"
  end
end

task :release_standalone => standalone_zip do
  load "bin/ocran"
  #sh "rubyforge add_release ocran ocran-standalone #{Ocran::VERSION} #{standalone_zip}"
end

file "bin/ocrasa.rb" => ["bin/ocra", "share/ocran/stub.exe", "share/ocran/stubw.exe", "share/ocran/lzma.exe", "share/ocran/edicon.exe"] do
  cp "bin/ocran", "bin/ocrasa.rb"
  File.open("bin/ocrasa.rb", "a") do |f|
    f.puts "__END__"

    stub = File.open("share/ocran/stub.exe", "rb") { |g| g.read }
    stub64 = [stub].pack("m")
    f.puts stub64.size
    f.puts stub64

    stub = File.open("share/ocran/stubw.exe", "rb") { |g| g.read }
    stub64 = [stub].pack("m")
    f.puts stub64.size
    f.puts stub64

    lzma = File.open("share/ocran/lzma.exe", "rb") { |g| g.read }
    lzma64 = [lzma].pack("m")
    f.puts lzma64.size
    f.puts lzma64

    lzma = File.open("share/ocran/edicon.exe", "rb") { |g| g.read }
    lzma64 = [lzma].pack("m")
    f.puts lzma64.size
    f.puts lzma64
  end
end

task :clean do
  rm_f Dir["{bin,samples}/*.exe"]
  rm_f Dir["share/ocran/{stub,stubw,edicon}.exe"]
  sh "ridk exec make -C src clean"
end

task :test_standalone => :standalone do
  ENV["TESTED_OCRAN"] = "ocransa.rb"
  system("rake test")
  ENV["TESTED_OCRAN"] = nil
end

task :release_docs => :redocs do
  sh "pscp -r doc/* larsch@ocran.rubyforge.org:/var/www/gforge-projects/ocran"
end

# vim: syntax=Ruby
