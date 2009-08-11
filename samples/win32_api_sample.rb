require 'win32/api'
exit if defined?(Ocra)
Win32::API.new('MessageBox', 'LPPI', 'I', 'user32').call(0, "Hello, World!", "Greeting", 0)
