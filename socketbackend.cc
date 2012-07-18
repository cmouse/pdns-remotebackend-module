#include "socketbackend.hh"

static const char *kBackendId = "[SocketBackend]";

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
