require 'srcrootlib'
exit 1 unless defined?(SrcRootLib)
File.read(File.join(File.dirname(__FILE__), "..", "lib", "srcrootlib.rb"))
