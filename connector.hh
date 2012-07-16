#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <map>
#include "pdns/misc.hh"
#include "json.h"

namespace Socketbackend {

using boost::asio::ip::udp;
using boost::asio::ip::tcp;
using boost::asio::local::stream_protocol;

class Connector {
 public:
   virtual ~Connector() {};
   virtual const std::string getConnectorName() { return ""; };
   virtual bool query(const std::string &request, std::string &result) {
      JsonNode *jquery = json_decode(request.c_str());
      JsonNode *jres = NULL;
      bool res = this->query(jquery,&jres);
      json_delete(jquery);
      if (res == false) return false;
      result = json_encode(jres);
      json_delete(jres);
      return true;
   };
   virtual bool query(const JsonNode *jquery, JsonNode **result) { 
      if (query(jquery) == false) return false;
      return reply(result);
   };
   virtual bool query(const std::string &request) { 
      JsonNode *query = json_decode(request.c_str());
      bool res = this->query(query);
      json_delete(query);
      return res;
   };
   virtual bool query(const JsonNode *jquery) { return false; };
   virtual bool reply(std::string &result) {
      JsonNode *jres = NULL;
      if (this->reply(&jres) == false) return false; 
      result = json_encode(jres); // FIXME: Should resulting char* be freed?
      json_delete(jres);
      return true;
   };
   virtual bool reply(JsonNode **result) { return false; };

   static Connector *build(const std::string &connstr);
 protected:
   Connector() { do_http = false; do_reuse = false; timeout = 2; };
   void initialize() {};
   std::string connstr;
   std::map<std::string, std::string> options;

   int timeout;
   bool do_http; // toggle to turn on full HTTP requests
   bool do_reuse; // toggle to turn on connection reuse (for TCP)
   boost::asio::io_service io_service;
};

class UnixConnector : public Connector {
 public:
   virtual const std::string getConnectorName() { return "UnixConnector"; };
   virtual bool query(const JsonNode *jquery);
   virtual bool reply(JsonNode **result);
 private:
   stream_protocol::endpoint *ep;
   stream_protocol::socket *socket;
};

class TCPConnector : public Connector {
 public:
   virtual const std::string getConnectorName() { return "TCPConnector"; };
   virtual bool query(const JsonNode *jquery);
   virtual bool reply(JsonNode **result);
 private:
   tcp::endpoint *ep;
   tcp::socket *socket;
};

class UDPConnector : public Connector {
 public:
   virtual const std::string getConnectorName() { return "UDPConnector"; };
   virtual bool query(const JsonNode *jquery);
   virtual bool reply(JsonNode **result);
 private:
   udp::endpoint *ep;
   udp::socket *socket;
};

};
