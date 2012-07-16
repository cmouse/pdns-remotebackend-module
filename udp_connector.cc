#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

#include "socketbackend.hh"

namespace Socketbackend {

using namespace boost::asio::ip::udp;
using namespace std;

void UDPConnector::reconnect() {
  if (connected) return;
  
  L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<opts["host"]<<":"<<opts["port"]<<endl;
  // check if path exists
´ 
  udp::endpoint ep(opts[host], opts[port]);
  socket = new udp::socket(io_service);
  socket->reuse_address = true;
  socket->connect(ep);
  
  connected = true; // shall be seen
}

};

bool UDPConnector::query(const std::string &request)
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

bool UDPConnector::reply(std::string &result)
{
  try {
    socket->read_some(result);
  } catch {
     connected = false;
     return false;
  }
  return true;
};

#endif
