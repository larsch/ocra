Dir.chdir(File.dirname(__FILE__))
raise "Can't find the file a/b/c" unless File.exist?("a/b/c")
