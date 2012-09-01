#!/usr/bin/ruby
require "rubygems"
require "bundler/setup"
require "webrick"
require "./dnsbackend"
require "./backend"

server = WEBrick::HTTPServer.new :Port => 62434

be = Handler.new("./remote.sqlite3") 

server.mount "/dns", DNSBackendHandler, be
trap('INT') { server.stop }
server.start
