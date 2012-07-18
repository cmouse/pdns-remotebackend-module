#include "socketbackend.hh"
#include <boost/foreach.hpp>
#include <sys/socket.h>
#include <netdb.h>

namespace Socketbackend {

using namespace std;

void TCPConnector::reconnect() {
  if (connected) return;
  
//  L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<options["host"]<<":"<<options["port"]<<endl;
}

};

