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
  SocketBackend(const std::string &suffix="")
  {
      this->connector = Socketbackend::Connector::build(getArg("connection-string"));
      this->d_dnssec = mustDo("dnssec");
  }

  void lookup(const QType &qtype, const std::string &qdomain, DNSPacket *pkt_p=0, int zoneId=-1)
  {
      struct JsonNode *query, *args;
      if (d_result != NULL) {
          throw new SBException("Lookup while get running");
      }

      query = json_mkobject();
      args = json_mkobject();

      json_append_member(query, "method", json_mkstring("lookup"));
      json_append_member(args, "qtype", json_mkstring(qtype.getName().c_str()));
      json_append_member(args, "qname", json_mkstring(qdomain.c_str()));
      if (pkt_p != NULL) {
        json_append_member(args, "remote", json_mkstring(pkt_p->getRemote().c_str()));
        json_append_member(args, "local", json_mkstring(pkt_p->getLocal().c_str()));
        json_append_member(args, "real-remote", json_mkstring(pkt_p->getRealRemote().toString().c_str()));
      }
      json_append_member(query, "parameters", args);

      if (connector->query(query, &d_result) == false) {
         L<<Logger::Error<<"Failed to query "<<connector->getConnectorName()<<endl;
      }
      json_delete(query);

      d_position = json_find_member(d_result, "result"); // should be an array... 
      // check for "false"
      if (d_position->tag == JSON_BOOL && d_position->bool_ == false) {
         // call failed
          json_delete(d_result);
          return;
      }

      if (d_position->tag != JSON_ARRAY) {  
         json_delete(d_result); // this cannot be NULL anymore.
         throw new SBException("Expected result array - got something else");
      }
  }

  bool get(DNSResourceRecord &rr) { 
      if (d_result == NULL) return false;

      // parse rr
      rr.qtype = json_value("qtype",d_position)->string_;
      rr.qname = json_value("qname",d_position)->string_;
      rr.ttl = static_cast<int>(json_value("ttl",d_position)->number_);
      rr.domain_id = static_cast<int>(json_value("domain_id",d_position)->number_);
      rr.priority = static_cast<int>(json_value("priority",d_position)->number_); 

      if (json_value("auth", d_position, d_dnssec))
        rr.auth = static_cast<int>(json_value("auth", d_position)->number_) != 0;
      else
        rr.auth = 1;

      if (json_value("scope_mask", d_position, true))
        rr.scopeMask = lexical_cast<uint8_t>(json_value("auth", d_position)->string_);

      d_position = d_position->next;

      if (d_position == NULL)
      {
         json_delete(d_result);
         d_result = NULL;
      }
 
      return true;
  }

  bool list(const std::string &target, int domain_id) { 
      struct JsonNode *query, *args;
      if (d_result != NULL) {
          throw new SBException("Lookup while get running");
      }

      query = json_mkobject();
      args = json_mkobject();

      json_append_member(query, "method", json_mkstring("list"));
      json_append_member(args, "target", json_mkstring(target.c_str()));
      json_append_member(args, "domain_id", json_mknumber(domain_id));

      json_append_member(query, "parameters", args);

      if (connector->query(query, &d_result) == false) {
         L<<Logger::Error<<"Failed to query "<<connector->getConnectorName()<<endl;
      }
      json_delete(query);

      d_position = json_find_member(d_result, "result"); 

      // check for "false"
      if (d_position->tag == JSON_BOOL && d_position->bool_ == false) {
         // call failed
          json_delete(d_result);
          return false;
      }
      if (d_position->tag != JSON_ARRAY) {
         json_delete(d_result); // this cannot be NULL anymore.
         throw new SBException("Expected result array - got something else");
      }

      return true;
  };

  ~SocketBackend() {};
  bool getSOA(const std::string &name, SOAData &soadata, DNSPacket *p=0) { return false; };
  bool getDomainMetadata(const std::string& name, const std::string& kind, std::vector<std::string>& meta) { return false; };
  bool getDomainKeys(const std::string& name, unsigned int kind, std::vector<DNSBackend::KeyData>& keys) { return false; };
  bool getTSIGKey(const std::string& name, std::string* algorithm, std::string* content) { return false; };
  bool getBeforeAndAfterNamesAbsolute(uint32_t id, const std::string& qname, std::string& unhashed, std::string& before, std::string& after) { return false; };
  bool getBeforeAndAfterNames(uint32_t id, const std::string& zonename, const std::string& qname, std::string& before, std::string& after) { return false; };

  bool getDomainInfo(const std::string &domain, DomainInfo &di) { return false; };

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
