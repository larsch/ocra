puts "-----"
puts Dir[File.dirname(__FILE__)+"/../**/*"]

$LOAD_PATH.unshift File.join(File.dirname($0), '..', 'lib')
require 'srcrootlib'
exit 1 unless defined?(SrcRootLib)

File.read(File.join(File.dirname(__FILE__), "..", "lib", "srcrootlib.rb"))
