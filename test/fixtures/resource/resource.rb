Dir.chdir(File.dirname(__FILE__))
exit 1 if File.read("resource.txt") != "someresource\n"
exit 2 if File.read("res/resource.txt") != "anotherresource\n"
