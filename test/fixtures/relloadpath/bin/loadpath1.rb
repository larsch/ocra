Dir.chdir(File.dirname(__FILE__))
$LOAD_PATH.unshift("../lib")
$LOAD_PATH.unshift("sub")
require "somelib"
require "sublib"
