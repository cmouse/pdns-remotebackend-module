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
#include "pdns/logger.hh"
#include "connector.hh"
#include <boost/asio.hpp>

class SocketBackend : public DNSBackend 
{
  public:
  SocketBackend(const string &suffix="");
  void lookup(const QType &qtype, const string &qdomain, DNSPacket *pkt_p=0, int zoneId=-1); 
  bool get(DNSResourceRecord &);
  bool list(const string &target, int domain_id);  
  ~SocketBackend(){};
  bool getSOA(const string &name, SOAData &soadata, DNSPacket *p=0);
  bool getDomainMetadata(const string& name, const std::string& kind, std::vector<std::string>& meta);
  bool getDomainKeys(const string& name, unsigned int kind, std::vector<DNSBackend::KeyData>& keys);
  bool getTSIGKey(const string& name, string* algorithm, string* content);
  bool getBeforeAndAfterNamesAbsolute(uint32_t id, const std::string& qname, std::string& unhashed, std::string& before, std::string& after);
  bool getBeforeAndAfterNames(uint32_t id, const std::string& zonename, const std::string& qname, std::string& before, std::string& after);
  bool getDomainInfo(const string &domain, DomainInfo &di);

  protected:
    std::string connstr;
    Socketbackend::Connector *connector; 
};
#endif
