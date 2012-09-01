#!/usr/bin/ruby

require 'json'
require 'sqlite3'

def rr(qname, qtype, content, ttl, priority = 0, auth = 1, domain_id = -1)
   {:qname => qname, :qtype => qtype, :content => content, :ttl => ttl.to_i, :priority => priority.to_i, :auth => auth.to_i, :domain_id => domain_id.to_i}
end

class Handler
   attr :db
 
   def initialize(dbpath)
     @db = SQLite3::Database.new dbpath
   end

   def do_initialize(*args)
     return true, "Test bench initialized"
   end

   def do_getbeforeandafternamesabsolute(args)
        return [{:before => nil, :after => nil, :unhashed => ''}, nil]
   end

   def do_getbeforeandafternames(args)
        return [{:before => nil, :after => nil, :unhashed => ''}, nil]
   end

   def do_getdomainkeys(args)
	if args["name"] == "example.com"
	 return [[{:flags => 257, :active => true, :content => "Private-key-format: v1.2
Algorithm: 5 (RSASHA1)
Modulus: qtOIeE+C8PbxYI3xQFdwRx/x/44X539sSg3rNO4eX+hWtrn+p+ieFgr9GMjvhHMdzbCWL1O
lc5bJxBXdhJ+kVn3P4YH9tuy8GifVczI1/kpe47BF0dlchmHVyS7cBwjI2ugPbow+RsUsdjT7FEj7u7s
6xDqdhw8GmoOImcHYEyE=
PublicExponent: AQAB
PrivateExponent: mmT/5kVvAp0ewnboApBE4XPlBGYIAuqvFCuKV1eUSniyzwpCOL5NC305DgqmOJ7
W5A5jXzkJw/QYPkrI4NJs82kdb9+9UHdT/B/5vUCOwjEtudI8aGIm01wGB/0QOMoKN9FiPkw3Q5ioAh4
+juUKBkazF9cXmF5KQPkGfT0oasU=
Prime1: 2Ewdws840KxTxict0npwsCTGpkcQ2oGPPrMkn29+ocrC2YqiNrUat//pwYCA6v+PKktofz12
frdljCXJbpJLLw==
Prime2: yi630Us/76OXUyEjS1vsVs9Pbay/SBkRr/sD3vw09libw7TEu/VeIlgvBNJnrtXhbBWyGaxZ
TVJEA5kEoXCyrw==
Exponent1: F2LzfNLHA9j+HdiynaVbddAkImpgqh+EC3V3Kj315Sx1MOxKabvfLbHf5moshjwnkJ0iq
U9N9pmBw0t6ohfzRQ==
Exponent2: x49rUkLTHca8A4pu2uAlm93OCXP77b8IzUmXHY6U/B6jyHzxvpBG1OFzr2+6dUCY2uVjQ
KH0FzS++0oa3vps8w==
Coefficient: lo1LspowL7kWYGWHfpBP63amoSc5UZtTCPvnIdjZooIZE0PedkzUyLOcUrxjMTqRC1Z
6pV/EiH5ej3j/LsO1mg=="}]]
	end
   end

   def do_lookup(args)
     ret = []
     loop do
        begin
          sargs = {}
          if (args["qtype"] == "ANY")
             sql = "SELECT * FROM records WHERE name = :qname"
             sargs["qname"] = args["qname"]
          else
             sql = "SELECT * FROM records WHERE name = :qname AND type = :qtype"
             sargs["qname"] = args["qname"]
             sargs["qtype"] = args["qtype"]
          end
          db.execute(sql, sargs) do |row|
            ret << rr(row[2], row[3], row[4], row[5], row[6], 1, row[1])
          end
        rescue Exception => e
          e.backtrace
        end
        break
     end
     return false unless ret.size > 0
     return [ret,nil]
   end
 
   def do_list(args)
     target = args["target"]
     ret = []
     loop do
        begin
          d_id = db.get_first_value("SELECT id FROM domains WHERE name = ?", target)
          return false if d_id.nil?
          db.execute("SELECT * FROM records WHERE domain_id = ?", d_id) do |row|
            ret << rr(row[2], row[3], row[4], row[5], row[6], 1, row[1])
          end
        rescue Exception => e
          e.backtrace
        end
        break
     end
     return false unless ret.size > 0
     return [ret,nil]
   end

   def do_getdomainmetadata(args) 
	return false
   end

   def do_setdomainmetadata(args)
	return false
   end
end
