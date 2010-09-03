Dir.chdir(File.dirname(__FILE__))
require "../lib/somelib"
require "sub/sublib" if RUBY_VERSION < "1.9.2"
