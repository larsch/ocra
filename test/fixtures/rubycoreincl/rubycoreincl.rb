require 'cgi'
File.open("output.txt", "w") do |f|
  f.write CGI::escapeHTML("3 < 5")
end
