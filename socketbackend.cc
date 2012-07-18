#include "socketbackend.hh"

static const char *kBackendId = "[SocketBackend]";

SocketBackend::SocketBackend(const std::string &suffix)
{
      setArgPrefix("socket"+suffix);
      this->connector = Socketbackend::Connector::build(getArg("connection-string"));
      this->d_dnssec = mustDo("dnssec");
      d_result = d_position = NULL;
      L<<Logger::Info<<"Created SocketBackend"<<endl;
}

SocketBackend::~SocketBackend() {
     if (connector != NULL) delete connector;
     if (d_result != NULL) json_delete(d_result);
}

void SocketBackend::lookup(const QType &qtype, const std::string &qdomain, DNSPacket *pkt_p, int zoneId)
{
      struct JsonNode *query, *args;

      L<<Logger::Info<<"Running lookup for "<<qdomain<<endl;

      if (d_result != NULL) {
          throw new AhuException("Lookup while get running");
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

      if (d_result == NULL) return; // it failed. 

      d_position = json_find_member(d_result, "result"); // should be an array...
      // check for "false"
      if (d_position->tag == JSON_BOOL && d_position->bool_ == false) {
         // call failed
          json_delete(d_result);
          d_result = NULL;
          return;
      }
      if (d_position->tag != JSON_ARRAY) {
         json_delete(d_result); 
         d_result = NULL;
         throw new AhuException("Expected result array - got something else");
      }

      // set it to first element
      d_position = json_first_child(d_position);
}
bool SocketBackend::get(DNSResourceRecord &rr) {
      if (d_result == NULL) return false;
      JsonNode *ptr;

      try {
      // parse rr
      rr.qtype = json_value("qtype",d_position)->string_;
      rr.qname = json_value("qname",d_position)->string_;
      rr.content = json_value("content",d_position)->string_;
      } catch (...) {
        L<<Logger::Error<<"Cannot parse result line a"<<endl;
      }

      try {
      rr.ttl = static_cast<int>(json_value("ttl",d_position)->number_);
      rr.domain_id = static_cast<int>(json_value("domain_id",d_position)->number_);
      rr.priority = static_cast<int>(json_value("priority",d_position)->number_);
      } catch (...) {
        L<<Logger::Error<<"Cannot parse result line b"<<endl;
      }

      try {
      if ((ptr = json_value("auth", d_position, !d_dnssec)) != NULL)
        rr.auth = static_cast<int>(ptr->number_) != 0;
      else
        rr.auth = 1;

      if ((ptr = json_value("scope_mask", d_position, true)) != NULL)
        rr.scopeMask = lexical_cast<uint8_t>(ptr->string_);

      } catch (...) {
        L<<Logger::Error<<"Cannot parse result line c"<<endl;
      }

      d_position = d_position->next;

      if (d_position == NULL)
      {
         json_delete(d_result);
         d_result = NULL;
      }

      return true;
}

bool SocketBackend::list(const std::string &target, int domain_id) {
      struct JsonNode *query, *args;
      if (d_result != NULL) {
          throw new AhuException("Lookup while get running");
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

      if (d_result == NULL) return false;

      d_position = json_find_member(d_result, "result");

      // check for "false"
      if (d_position->tag == JSON_BOOL && d_position->bool_ == false) {
         // call failed
          json_delete(d_result);
          d_result = NULL;
          return false;
      }
      if (d_position->tag != JSON_ARRAY) {
         json_delete(d_result); // this cannot be NULL anymore.
         d_result = NULL;
         throw new AhuException("Expected result array - got something else");
      }

      return true;
};


DNSBackend *SocketBackend::maker()
{
   try {
      return new SocketBackend();
   }
   catch(...) {
      L<<Logger::Error<<kBackendId<<" Unable to instantiate a socketbackend!"<<endl;
      return 0;
   };
}

class SocketBackendFactory : public BackendFactory
{
  public:
      SocketBackendFactory() : BackendFactory("socket") {}

      void declareArguments(const std::string &suffix="")
      {
          declare(suffix,"dnssec","Enable dnssec support","no");
          declare(suffix,"connection-string","Connection string",""); 
      }

      DNSBackend *make(const std::string &suffix="")
      {
         return new SocketBackend(suffix);
      }
};

class SocketLoader
{
   public:
      SocketLoader()
      {
         BackendMakers().report(new SocketBackendFactory);
         L<<Logger::Notice<<kBackendId<<" This is the socketbackend version "VERSION" ("__DATE__", "__TIME__") reporting"<<endl;
      }
};

static SocketLoader socketloader;
