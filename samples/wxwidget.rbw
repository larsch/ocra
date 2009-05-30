require 'rubygems'
require 'wx'

class MyApp < Wx::App
  def on_init
    @frame = Wx::Frame.new( nil, -1, "Application" )
    @frame.show
  end
end

# Create MyApp
app = MyApp.new

# Run MyApp (unless Ocra is currently defined, and we are compiling
# the application).
app.main_loop unless defined?(Ocra)
