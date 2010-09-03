$LOAD_PATH.unshift(File.expand_path("../lib", File.dirname(__FILE__)))
$LOAD_PATH.unshift(File.expand_path("sub", File.dirname(__FILE__)))
require "somelib"
require "sublib"
