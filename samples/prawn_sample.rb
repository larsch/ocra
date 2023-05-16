require 'prawn'
exit if defined?(Ocran)
Prawn::Document.generate('prawn_sample.pdf') do
  text "Hello, World!"
  font.instance_eval { find_font("Helvetica.afm") } or fail
end
File.unlink("prawn_sample.pdf")
