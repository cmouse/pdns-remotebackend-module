#ifndef SOCKETBACKEND_HH
#define SOCKETBACKEND_HH

#include <string>
#include <map>
#include <sys/types.h>
#include <regex.h>
#include <boost/shared_ptr.hpp>
#include "pdns/namespaces.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dns.hh"
#include "pdns/dnspacket.hh"
#include "pdns/logger.hh"
#include "connector.hh"
#include "json.h"

using namespace std;

class SocketBackend : public DNSBackend 
{
  public:
  SocketBackend(const std::string &suffix="")
  {
      this->connector = Socketbackend::Connector::build(getArg("connection-string"));
  }

  void lookup(const QType &qtype, const std::string &qdomain, DNSPacket *pkt_p=0, int zoneId=-1)
  {
      struct JsonNode *query;
      query = json_mkobject();
      json_append_member(query, "type", json_mkstring("query"));
      json_append_member(query, "qtype", json_mkstring(qtype.getName().c_str()));
      json_append_member(query, "qname", json_mkstring(qdomain.c_str()));
      if (pkt_p != NULL) {
        json_append_member(query, "remote", json_mkstring(pkt_p->getRemote().c_str()));
        json_append_member(query, "local", json_mkstring(pkt_p->getLocal().c_str()));
        json_append_member(query, "real-remote", json_mkstring(pkt_p->getRealRemote().toString().c_str()));
      }
      if (connector->query(query) == false) {
         L<<Logger::Error<<"Failed to query "<<connector->getConnectorName()<<endl;
      }
      json_delete(query);
  }

  bool get(DNSResourceRecord &rr) { return false; };
  bool list(const std::string &target, int domain_id) { return false; };
  ~SocketBackend() {};
  bool getSOA(const std::string &name, SOAData &soadata, DNSPacket *p=0) { return false; };
  bool getDomainMetadata(const std::string& name, const std::string& kind, std::vector<std::string>& meta) { return false; };
  bool getDomainKeys(const std::string& name, unsigned int kind, std::vector<DNSBackend::KeyData>& keys) { return false; };
  bool getTSIGKey(const std::string& name, string* algorithm, string* content) { return false; };
  bool getBeforeAndAfterNamesAbsolute(uint32_t id, const std::string& qname, std::string& unhashed, std::string& before, std::string& after) { return false; };
  bool getBeforeAndAfterNames(uint32_t id, const std::string& zonename, const std::string& qname, std::string& before, std::string& after) { return false; };

  bool getDomainInfo(const std::string &domain, DomainInfo &di) { return false; };

  protected:
    Socketbackend::Connector *connector; 
    bool d_locked; // locked in lookup/get
};
#endif
