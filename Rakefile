# -*- ruby -*-

require 'rubygems'
require 'hoe'
require './lib/ocra.rb'

Hoe.new('ocra', Ocra::VERSION) do |p|
  # p.rubyforge_name = 'ocrax' # if different than lowercase project name
  p.developer('Lars Christensen', 'larsch@belunktum.dk')
end


file 'share/ocra/stub.exe' => 'src/stub.exe' do
  mv 'src/stub.exe', 'share/ocra/stub.exe'
end

file 'src/stub.exe' do
  chdir 'src' do
    system("mingw32-make")
  end
end

task :stub => 'share/ocra/stub.exe'

task :test => :stub

# vim: syntax=Ruby
