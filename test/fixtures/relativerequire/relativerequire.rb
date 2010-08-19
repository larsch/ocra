$:.unshift File.dirname(__FILE__)
require 'somedir/somefile'
require 'SomeDir/otherfile'
exit 160 if __FILE__ == $0 and defined?(SomeConst) and defined?(OtherConst)
