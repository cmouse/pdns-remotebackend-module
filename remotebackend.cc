#include "remotebackend.hh"
#include <boost/foreach.hpp>

static const char *kBackendId = "[RemoteBackend]";

bool Connector::send(Json::Value &value) {
    return send_message(value);
}

bool Connector::recv(Json::Value &value) {
    Json::Value input;
    if (recv_message(input)>0) {
       bool rv = true;
       // check for error
       value = input.get("result",Json::Value());
       if (value.isNull() || (value.isBool() && value.asBool() == false)) {
           rv = false;
        }
        Json::Value messages = input.get("log", Json::Value());
        if (messages.isArray()) {
           // log em all
           for(Json::ValueIterator iter = messages.begin(); iter != messages.end(); iter++) {
              L<<Logger::Info<<"[remotebackend]:"<< (*iter).asString()<<std::endl;
           }
        }
        return rv;
    }
    return false;
}

RemoteBackend::RemoteBackend(const std::string &suffix)
{
      setArgPrefix("remote"+suffix);
      build(getArg("connection-string"));
      this->d_dnssec = mustDo("dnssec");
      this->d_index = -1;
}

RemoteBackend::~RemoteBackend() {
     if (connector != NULL) {
 	delete connector;
     }
}

int RemoteBackend::build(const std::string &connstr) {
      std::vector<std::string> parts;
      std::string type;
      std::string opts;
      std::map<std::string, std::string> options;

      // connstr is of format "type:options"
      size_t pos;
      pos = connstr.find_first_of(":");
      if (pos == std::string::npos)
         throw new AhuException("Invalid connection string: malformed");

      type = connstr.substr(0, pos);
      opts = connstr.substr(pos+1);

      stringtok(parts, opts, ",");

      // find out some options and parse them while we're at it
      BOOST_FOREACH(std::string opt, parts) {
          std::string key,val;
          if (opt.find_first_not_of(" ") == std::string::npos) continue;

          pos = opt.find_first_of("=");
          if (pos == std::string::npos) {
             throw new AhuException("Invalid connection string: option format is key=value");
          }
          key = opt.substr(0,pos);
          val = opt.substr(pos+1);
          options[key] = val;
      }

      if (type == "http") {
        this->connector = new TCPConnector(options);
      } else if (type == "unix") {
        this->connector = new UnixsocketConnector(options);
      } else if (type == "pipe") {
        this->connector = new PipeConnector(options);
      } else {
        throw new AhuException("Invalid connection string: unknown connector");
      }

      return -1;
}

void RemoteBackend::lookup(const QType &qtype, const std::string &qdomain, DNSPacket *pkt_p, int zoneId) {
   Json::Value query,args;

   if (d_index != -1) 
      throw AhuException("Attempt to lookup while one running");
 
   args["qtype"] = qtype.getName();
   args["qname"] = qdomain;
   if (pkt_p != NULL) {
     args["remote"] = pkt_p->getRemote();
     args["local"] = pkt_p->getLocal();
     args["real-remote"] = pkt_p->getRealRemote().toString();
   }

   query["method"] = "lookup";
   query["parameters"] = args;

   if (connector->send(query) == false || connector->recv(d_result) == false)  return;

   // OK. we have result values in result
   if (d_result.isArray() == false) return;
   d_index = 0;
}

bool RemoteBackend::get(DNSResourceRecord &rr) {
   if (d_index == -1) return false;
   
   Json::Value empty("");
   Json::Value emptyint(-1);

   rr.qtype = d_result[d_index].get("qtype",empty).asString();
   rr.qname = d_result[d_index].get("qname",empty).asString();
   rr.content = d_result[d_index].get("content",empty).asString();
   rr.ttl = d_result[d_index].get("ttl",emptyint).asInt();
   rr.domain_id = d_result[d_index].get("domain_id",emptyint).asInt();
   rr.priority = d_result[d_index].get("priority",emptyint).asInt();

   rr.scopeMask = d_result[d_index].get("scopeMask",Json::Value(0)).asInt();

   d_index++;
   if (d_index == d_result.size()) {
     d_result = Json::Value();
     d_index = -1;
   }
   return true;
}

bool RemoteBackend::list(const std::string &target, int domain_id) {
   Json::Value query;

   if (d_index != -1) 
      throw AhuException("Attempt to lookup while one running");

   query["method"] = "list";
   query["parameters"] = Json::Value();
   query["parameters"]["target"] = target;
   query["parameters"]["domain_id"] = domain_id;

   if (connector->send(query) == false || connector->recv(d_result) == false) 
     return false;
   d_index = 0;

   return true;
}

bool RemoteBackend::getBeforeAndAfterNamesAbsolute(uint32_t id, const std::string& qname, std::string& unhashed, std::string& before, std::string& after) {
   Json::Value query,answer;

   query["method"] = "getBeforeAndAfterNamesAbsolute";
   query["parameters"] = Json::Value();
   query["parameters"]["id"] = id;
   query["parameters"]["qname"] = qname;

   if (connector->send(query) == false || connector->recv(answer) == false)
     return false;
   
   unhashed = answer["result"]["unhashed"].asString();
   before = answer["result"]["before"].asString();
   after = answer["result"]["after"].asString();
  
   return true;
}

bool RemoteBackend::getBeforeAndAfterNames(uint32_t id, const std::string& zonename, const std::string& qname, std::string& before, std::string& after) {
   Json::Value query,answer;

   query["method"] = "getBeforeAndAfterNames";
   query["parameters"] = Json::Value();
   query["parameters"]["id"] = id;
   query["parameters"]["zonename"] = zonename;
   query["parameters"]["qname"] = qname;

   if (connector->send(query) == false || connector->recv(answer) == false)
     return false;

   before = answer["result"]["before"].asString();
   after = answer["result"]["after"].asString();

   return true;
}

bool RemoteBackend::getDomainMetadata(const std::string& name, const std::string& kind, std::vector<std::string>& meta) {
   Json::Value query,answer;
   query["method"] = "getDomainMetadata";
   query["parameters"] = Json::Value();
   query["parameters"]["name"] = name;
   query["parameters"]["kind"] = kind;
   if (connector->send(query) == false || connector->recv(answer) == false)
     return false;

   meta.clear();

   for(Json::ValueIterator iter = answer["result"].begin(); iter != answer["result"].end(); iter++) {
          meta.push_back((*iter).asString());
   }

   return true;
}


bool RemoteBackend::getDomainKeys(const std::string& name, unsigned int kind, std::vector<DNSBackend::KeyData>& keys) {
   Json::Value query,answer;
   query["method"] = "getDomainKeys";
   query["parameters"] = Json::Value();
   query["parameters"]["name"] = name;
   query["parameters"]["kind"] = kind;

   if (connector->send(query) == false || connector->recv(answer) == false)
     return false;

   keys.clear();

   for(Json::ValueIterator iter = answer["result"].begin(); iter != answer["result"].end(); iter++) {
      DNSBackend::KeyData key;
      key.id = (*iter)["id"].asUInt();
      key.flags = (*iter)["flags"].asUInt();
      key.active = (*iter)["active"].asBool();
      key.content = (*iter)["content"].asString();
   }

   return true;
}

bool RemoteBackend::getTSIGKey(const std::string& name, std::string* algorithm, std::string* content) {
   Json::Value query,answer;
   query["method"] = "getTSIGKey";
   query["parameters"] = Json::Value();
   query["parameters"]["name"] = name;

   if (connector->send(query) == false || connector->recv(answer) == false)
     return false;

   if (algorithm != NULL)
     algorithm->assign(answer["result"]["algorithm"].asString());
   if (content != NULL)
     content->assign(answer["result"]["content"].asString());
   
   return true;
}

DNSBackend *RemoteBackend::maker()
{
   try {
      return new RemoteBackend();
   }
   catch(...) {
      L<<Logger::Error<<kBackendId<<" Unable to instantiate a remotebackend!"<<endl;
      return 0;
   };
}

class RemoteBackendFactory : public BackendFactory
{
  public:
      RemoteBackendFactory() : BackendFactory("remote") {}

      void declareArguments(const std::string &suffix="")
      {
          declare(suffix,"dnssec","Enable dnssec support","no");
          declare(suffix,"connection-string","Connection string","");
      }

      DNSBackend *make(const std::string &suffix="")
      {
         return new RemoteBackend(suffix);
      }
};

class RemoteLoader
{
   public:
      RemoteLoader()
      {
         curl_global_init(CURL_GLOBAL_ALL);
         BackendMakers().report(new RemoteBackendFactory);
         L<<Logger::Notice<<kBackendId<<" This is the remotebackend version "VERSION" ("__DATE__", "__TIME__") reporting"<<endl;
      }
};

static RemoteLoader remoteloader;
