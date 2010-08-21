exit if defined?(Ocra)
exit 1 if ARGV.size != 2
exit 2 if ARGV[0] != "foo"
if RUBY_VERSION == "1.8.6"
  # Ruby 1.8.6 has a command line quote-parsing bug that leaves extra
  # chars after the argument
  exit 3 if ARGV[1].index("bar baz \"quote\"") != 0
else
  exit 3 if ARGV[1] != "bar baz \"quote\""
end
exit 5
