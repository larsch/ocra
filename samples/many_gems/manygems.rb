# frozen_string_literal: true

require 'sqlite3'

if Gem.win_platform?
  require 'tzinfo'
  require 'tzinfo/data'
end

require 'xmlsimple'
require 'psych'
require 'win32/service'
require 'win32/process'
require 'rufus/scheduler'
require 'net/ftp'
require 'net/ftp/list'
require 'ffi'
require 'logger'
require 'rest-client'
require 'sequel'
require 'addressable'
require 'roo'
require 'mini_magick'
require 'openssl'
require 'openssl/win/root'
require 'ruby-measurement'
require 'loofah'
require 'lumberjack'
require 'tiny_tds'
require 'tiny_tds/tiny_tds'

# Your application code goes here
puts "All gems have been successfully loaded!"

# Example usage of some gems
puts TinyTds::VERSION
puts SQLite3::VERSION
puts TZInfo::VERSION if Gem.win_platform?
puts Rufus::Scheduler::VERSION
puts Net::FTP::VERSION
puts OpenSSL::VERSION
#puts Lumberjack::VERSION

# Connect to an in-memory SQLite database
db = SQLite3::Database.new(':memory:')
