#include <string>
#include <vector>
#include <map>
#include "json.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <sstream>
#include "pdns/namespaces.hh"
#include "pdns/logger.hh"

#define L theL()

namespace Socketbackend {

  class Connector {
   public:
     Connector();
     virtual ~Connector() {};
     virtual const std::string getConnectorName() { return ""; };
     bool query(const std::string &request, std::string &result);
     bool query(JsonNode *request, JsonNode **result);
     bool query(JsonNode *request);
     bool reply(JsonNode **result);
     bool query(const std::string &request);
     bool reply(std::string &result);
     static Connector *build(const std::string &connstr);

   protected:
     virtual void reconnect() {};
     std::string connstr;
     std::map<std::string, std::string> options;
  
     bool connected;
     int timeout;
     bool do_reuse;
     int sock;
  };
  
  class UnixConnector : public Connector {
   public:
     UnixConnector() { connected = false; };
     ~UnixConnector();
     virtual const std::string getConnectorName() { return "UnixConnector"; };
   protected:
     virtual void reconnect();
  };
  
  class TCPConnector : public Connector {
   public:
     TCPConnector() { connected = false; };
     ~TCPConnector() { if (connected) { close(sock); } }
     virtual const std::string getConnectorName() { return "TCPConnector"; };
   protected:
     virtual void reconnect();
  };

};
