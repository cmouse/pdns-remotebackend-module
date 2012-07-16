#include "socketbackend.hh"
#include "json.h"

static const char *kBackendId = "[SocketBackend]";

class SocketBackendFactory : public BackendFactory
{
  public:
      SocketBackendFactory() : BackendFactory("socket") {}

      void declareArguments(const string &suffix="")
      {
      }

      DNSBackend *make(const string &suffix="")
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
