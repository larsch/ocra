require 'rubygems'
gem 'sys-proctable'
require 'sys/proctable'
require 'time'
include Sys

# Everything
ProcTable.ps{ |p|
   puts p.pid.to_s + "    " + p.comm
}
