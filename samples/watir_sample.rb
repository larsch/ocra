#!/usr/bin/ruby
require "watir"
exit if defined?(Ocra)

test_site = "http://www.google.com"

ie = Watir::IE.new

puts "Beginning of test: Google search."
puts " Step 1: go to the test site: " + test_site
ie.goto test_site

puts " Step 2: enter 'pickaxe' in the search text field."
ie.text_field(:name, "q").set "pickaxe" # "q" is the name of the search field

puts " Step 3: click the 'Google Search' button."
ie.button(:name, "btnG").click # "btnG" is the name of the Search button

puts " Expected Result:"
puts "  A Google page with results should be shown. 'Programming Ruby' should be high on the list."

puts " Actual Result:"
if ie.text.include? "Programming Ruby"
  puts "  Test Passed. Found the test string: 'Programming Ruby'. Actual Results match Expected Results."
else
  puts "  Test Failed! Could not find: 'Programming Ruby'."
end

ie.close
