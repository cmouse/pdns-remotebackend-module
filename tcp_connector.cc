#include "socketbackend.hh"
#include <boost/foreach.hpp>

namespace Socketbackend {

using boost::asio::ip::tcp;
using namespace std;

void TCPConnector::reconnect() {
  if (connected) return;
  
  L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<options["host"]<<":"<<options["port"]<<endl;
  // check if path exists
 
  tcp::resolver resolver(io_service);
  tcp::resolver::query query(options["host"], options["port"]);
  tcp::resolver::iterator iterator = resolver.resolve(query);
  tcp::resolver::iterator end;
  boost::asio::socket_base::reuse_address so_reuse_addr(true);
  
  socket = new tcp::socket(io_service);
  socket->set_option(so_reuse_addr);

  while(iterator != end) {
     boost::system::error_code ec;
     socket->close();
     socket->connect(*iterator++,ec);
     if (!ec) break;
  }
  
  if (socket->is_open() == false) {
     L<<Logger::Error<<"Cannot connect to remote end"<<endl;
     connected = false;
  } else {
     L<<Logger::Info<<"Connection successful"<<endl;
     connected = true; // shall be seen
  }
}

bool TCPConnector::query(const std::string &request)
{
  reconnect();
  if (!connected) return false;
  try {
     socket->send(boost::asio::buffer(request));
  } catch(exception) {
     connected = false;
     return false;
  }
  return true;
};

bool TCPConnector::reply(std::string &result)
{
  char data[1500];
  if (!connected) return false;
  try {
     socket->read_some(boost::asio::buffer(data));
     result = data;
  } catch(exception) {
     connected = false;
     return false;
  }
  return true;
};

};

