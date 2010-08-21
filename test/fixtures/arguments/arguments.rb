exit if defined?(Ocra)
exit 1 if ARGV.size != 2
exit 2 if ARGV[0] != "foo"
exit 3 if ARGV[1] != "bar baz \"quote\""
exit 5
