require 'rubygems'
require 'wx'

# Main window frame with a button.
class MyMainWindow < Wx::Frame
  MY_BUTTON_ID = 1001
  def initialize
    super(nil, -1, "OCRA wxWidgets sample application")
    @button = Wx::Button.new(self, MY_BUTTON_ID, "OCRA Sample")
    evt_button(MY_BUTTON_ID) { close }
  end
end

# The sample application.
class MyApp < Wx::App
  def on_init
    @frame = MyMainWindow.new
    @frame.show
  end
end

# Create MyApp
app = MyApp.new

# Run MyApp (unless Ocra is currently defined, and we are compiling
# the application).
app.main_loop unless defined?(Ocra)
