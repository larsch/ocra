require 'rubygems'
# https://github.com/rubygems/bundler/issues/6937
# however, 'bundler/setup' causes a problem per se
# this helps, of cause:
# gem install --default bundler
# gem update --system
# bundler update --bundler
require 'bundler'
#  requires 'bundler/setup'
require 'rake'

# Just make sure the constant exists
Rake
  