#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

#include "socketbackend.hh"

namespace Socketbackend {

using namespace boost::asio::local;
using namespace std;

void UnixConnector::reconnect() {
  if (connected) return;
  
  L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<options["path"]<<endl;
  // check if path exists
´ 
  local::stream_protocol::endpoint ep(options[path]);
  socket = new local::stream_protocol::socket(io_service);
  socket->reuse_address = true;
  socket->connect(ep);
  
  connected = true; // shall be seen
}

bool UnixConnector::query(const std::string &request)
{
  reconnect();
  try {
     socket->write_data(request);
  } catch {
     connected = false;
     return false;
  }
  return true;
};

bool UnixConnector::reply(std::string &result)
{
  try {
    socket->read_some(result);
  } catch {
     connected = false;
     return false;
  }
  return true;
};

};

#endif
