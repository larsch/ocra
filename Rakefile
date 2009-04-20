# -*- ruby -*-

require 'rubygems'
require 'hoe'
require './lib/ocra.rb'

Hoe.new('ocra', Ocra::VERSION) do |p|
  p.developer('Lars Christensen', 'larsch@belunktum.dk')
end

task :stub do
  sh "mingw32-make -C src"
  cp 'src/stub.exe', 'share/ocra/stub.exe'
end

task :test => :stub

task :standalone => [ 'bin/ocrasa.rb', 'bin/ocrasa.exe' ]

task :release_standalone => :standalone do
  sh "rubyforge add_release ocra ocra-standalone #{Ocra::VERSION} bin/ocrasa.rb"
end

file 'bin/ocrasa.rb' => [ 'bin/ocra.rb', 'share/ocra/stub.exe', 'share/ocra/lzma.exe' ] do
  cp 'bin/ocra.rb', 'bin/ocrasa.rb'
  File.open("bin/ocrasa.rb", "a") do |f|
    f.puts "__END__"
    stub = File.open("share/ocra/stub.exe", "rb") {|g| g.read}
    stub64 = [stub].pack("m")
    f.puts stub64.size
    f.puts stub64
    lzma = File.open("share/ocra/lzma.exe", "rb") {|g| g.read}
    lzma64 = [lzma].pack("m")
    f.puts lzma64.size
    f.puts lzma64
  end
end

file 'bin/ocrasa.exe' => [ 'bin/ocra.rb', 'bin/ocrasa.rb' ] do
  sh "ruby bin/ocra.rb bin/ocrasa.rb"
end

task :clean do
  rm_rf Dir.glob("bin/*.exe")
  sh "mingw32-make -C src clean"
end

# vim: syntax=Ruby
