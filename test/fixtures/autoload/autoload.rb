$:.unshift File.dirname(__FILE__)
autoload :Foo, 'foo'
Foo if __FILE__ == $0
