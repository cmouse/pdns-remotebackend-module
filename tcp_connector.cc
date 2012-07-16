#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS

#include "socketbackend.hh"

namespace Socketbackend {

using namespace boost::asio::ip::tcp;
using namespace std;

void TCPConnector::reconnect() {
  if (connected) return;
  
  L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<opts["host"]<<":"<<opts["port"]<<endl;
  // check if path exists
´ 
  tcp::endpoint ep(opts[host], opts[port]);
  socket = new tcp::socket(io_service);
  socket->reuse_address = true;
  socket->connect(ep);
  
  connected = true; // shall be seen
}

};

bool TCPConnector::query(const std::string &request)
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

bool TCPConnector::reply(std::string &result)
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
