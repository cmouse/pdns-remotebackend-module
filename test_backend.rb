#!/usr/bin/ruby
require 'json'

$stdin.each do |line|
  begin
    input = JSON.parse(line)
  rescue
    next
  end

  STDERR.puts "Received #{input["method"]}"

  if (input["method"] == "initialize") 
    output = {"result" => true}
    output["log"] = ["test backend v1.0 initialized"]
  elsif (input["method"] == "lookup")
   if (input["parameters"]["qname"] == "tdc.fi")
     output = {"result" => [
        {"qtype" => "SOA", "qname" => "tdc.fi", "ttl" => 60, "content" => "ns-fi.sn.net dns.tdc.fi 2012071700 28800 7200 1209600 86400", "priority" => 0, "domain_id" => -1}]}
      if (input["parameters"]["qtype"] == "ANY")
        output["result"]<<{"qtype" => "NS", "qname" => "tdc.fi", "ttl" => 60, "content" => "ns-fi.sn.net", "priority" => 0, "domain_id" => -1}
      end
    elsif (input["parameters"]["qname"] == "www.tdc.fi" and input["parameters"]["qtype"] == "ANY")
      output = {"result" => [
        {"qtype" => "A", "qname" => "www.tdc.fi", "ttl" => 60, "content" => "127.0.0.1", "priority" => 0, "domain_id" => -1},
      ]}
    else
      output = {"result" => false}
    end
  else
    output = {"result" => false}
  end

  puts output.to_json

  STDERR.puts output.to_json
  STDOUT.flush
end

