# connecting to a microsoft sql server
# if gem install tiny_tds did not work try:
# gem install tiny_tds --with-freetds-include=C:\Ruby31-x64\msys64\ucrt64\include\freetds
require "tiny_tds"

# remember to turn on tcp/ip in the sql server configuration manager before you try to connect
# Press Windows button and R and open sqlservermanager15.msc
client = TinyTds::Client.new username: 'sa', password: 'sasa', host: '127.0.0.1', port: '49691'

#sleep 202