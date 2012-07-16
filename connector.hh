#include <string>
#include <vector>
#include <map>
#include "json.h"

namespace Socketbackend {

  class Connector {
   public:
     Connector();
     virtual ~Connector() {};
     virtual const std::string getConnectorName() { return ""; };
     bool query(const std::string &request, std::string &result);
     bool query(const JsonNode *jquery, JsonNode **result);
     bool query(const JsonNode *jquery);
     bool reply(JsonNode **result);
     virtual bool query(const std::string &request) { return false; };
     virtual bool reply(std::string &result) { return false; };
     static Connector *build(const std::string &connstr);

   protected:
     void initialize() {};
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
     ~UnixConnector() { if (connected) { close(sock); } }
     virtual const std::string getConnectorName() { return "UnixConnector"; };
     virtual bool query(const std::string &request);
     virtual bool reply(std::string &result);
   private:
     void reconnect();
  };
  
  class TCPConnector : public Connector {
   public:
     TCPConnector() { connected = false; };
     ~TCPConnector() { if (connected) { close(sock); } }
     virtual const std::string getConnectorName() { return "TCPConnector"; };
     virtual bool query(const std::string &request);
     virtual bool reply(std::string &result);
   private:
     void reconnect();
  };

};
