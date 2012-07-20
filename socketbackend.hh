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

using namespace std;

class SBException {
public:
  SBException(const string &reason)
  {
      d_reason=reason;
  }

  string txtReason()
  {
    return d_reason;
  }
private:
  string d_reason;
};

class SocketBackend : public DNSBackend 
{
  public:
  SocketBackend(const std::string &suffix="");
  ~SocketBackend(); 

  void lookup(const QType &qtype, const std::string &qdomain, DNSPacket *pkt_p=0, int zoneId=-1);
  bool get(DNSResourceRecord &rr); 
  bool list(const std::string &target, int domain_id); 

  // unimplemented
  virtual bool getDomainMetadata(const std::string& name, const std::string& kind, std::vector<std::string>& meta);
  virtual bool getDomainKeys(const std::string& name, unsigned int kind, std::vector<DNSBackend::KeyData>& keys);
  virtual bool getTSIGKey(const std::string& name, std::string* algorithm, std::string* content);
  virtual bool getBeforeAndAfterNamesAbsolute(uint32_t id, const std::string& qname, std::string& unhashed, std::string& before, std::string& after);
  virtual bool getBeforeAndAfterNames(uint32_t id, const std::string& zonename, const std::string& qname, std::string& before, std::string& after);

  static DNSBackend *maker();

  protected:
    Socketbackend::Connector *connector; 
    bool d_dnssec;
    JsonNode *d_result; // result from lookup or list
    JsonNode *d_position; // position in result
 
  private:
    JsonNode *json_value(const std::string member, JsonNode *data, bool null_ok = false) { 
      data = json_find_member(data, member.c_str());
      if (data == NULL) {
        if (null_ok) return NULL;
        throw new SBException("Got NULL when one was not expected");
      }
      return data;
    };
};
#endif
