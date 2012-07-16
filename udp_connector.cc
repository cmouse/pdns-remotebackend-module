#include "socketbackend.hh"
#include <boost/foreach.hpp>

namespace Socketbackend {

using boost::asio::ip::udp;
using namespace std;

void UDPConnector::reconnect() {
  if (connected) return;
  // check if path exists
 
  udp::resolver resolver(io_service);
  udp::resolver::query query(options["host"], options["port"]);
  iterator = resolver.resolve(query);
  udp::resolver::iterator end;
  boost::asio::socket_base::reuse_address so_reuse_addr(true);
  
  socket = new udp::socket(io_service);
  socket->set_option(so_reuse_addr);
  
  if (iterator != end) {
    connected = true;
  } else {
    throw new AhuException("Cannot resolve any endpoint");
    connected = false;
  }
}

bool UDPConnector::query(const std::string &request)
{
  reconnect();
  if (!connected) return false;
  try {
     socket->send_to(boost::asio::buffer(request), *iterator);
  } catch(exception) {
     connected = false;
     return false;
  }
  return true;
};

bool UDPConnector::reply(std::string &result)
{
  char data[1500];
  if (!connected) return false;
  try {
     socket->receive(boost::asio::buffer(data));
     result = data;
  } catch(exception) {
     connected = false;
     return false;
  }
  return true;
};

};

