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
   bool query(const std::string &request, std::string &result);
   bool query(const JsonNode *jquery, JsonNode **result);
   bool query(const JsonNode *jquery);
   bool reply(JsonNode **result);

   virtual bool query(const std::string &request);
   virtual bool reply(std::string &result);

   static Connector *build(const std::string &connstr);
 protected:
   Connector() { do_reuse = false; timeout = 2; };
   void initialize() {};
   std::string connstr;
   std::map<std::string, std::string> options;

   int timeout;
   bool do_reuse; // toggle to turn on connection reuse (for TCP)
   boost::asio::io_service io_service;
};

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

class UnixConnector : public Connector {
 public:
   UnixConnector(): Connector() { connected = false; };
   ~UnixConnector() { if (connected) { socket->close(); }; delete socket; }
   virtual const std::string getConnectorName() { return "UnixConnector"; };
   virtual bool query(const std::string &request);
   virtual bool reply(std::string &result);
 private:
   void reconnect();
   stream_protocol::socket *socket;
   bool connected;
};

#endif

class TCPConnector : public Connector {
 public:
   virtual const std::string getConnectorName() { return "TCPConnector"; };
   virtual bool query(const std::string &request);
   virtual bool reply(std::string &result);
 private:
   void reconnect();
   bool connected;
   tcp::socket *socket;
};

class UDPConnector : public Connector {
 public:
   virtual const std::string getConnectorName() { return "UDPConnector"; };
   virtual bool query(const std::string &request);
   virtual bool reply(std::string &result);
 private:   
   void reconnect();
   udp::socket *socket;
   udp::resolver::iterator iterator;
   bool connected;
};

};
