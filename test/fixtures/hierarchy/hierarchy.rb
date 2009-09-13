Dir.chdir File.dirname(__FILE__)
fail unless File.exist? "assets/resource1.txt"
fail unless File.read("assets/resource1.txt") == "resource1\n"
fail unless File.exist? "assets/subdir/resource2.txt"
fail unless File.read("assets/subdir/resource2.txt") == "resource2\n"

