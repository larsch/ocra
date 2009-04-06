# -*- ruby -*-

require 'rubygems'
require 'hoe'
require './lib/ocra.rb'

Hoe.new('ocra', Ocra::VERSION) do |p|
  # p.rubyforge_name = 'ocrax' # if different than lowercase project name
  p.developer('Lars Christensen', 'larsch@belunktum.dk')
end

task :stub do
  sh "mingw32-make -C src"
  cp 'src/stub.exe', 'share/ocra/stub.exe'
end

task :test => :stub

# vim: syntax=Ruby
