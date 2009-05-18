if $0 == __FILE__
  exit 1 if ARGV.size != 2
  exit 2 if ARGV[0] != "foo"
  exit 3 if ARGV[1] != "bar baz"
  exit 5
end
