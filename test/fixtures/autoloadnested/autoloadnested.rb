$:.unshift File.dirname(__FILE__)
module Bar
  autoload :Foo, 'foo'
end
Bar::Foo if __FILE__ == $0
