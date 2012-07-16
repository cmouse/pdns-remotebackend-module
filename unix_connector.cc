#include "connector.hh"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/select.h>
#include <linux/un.h>
#include <cstring>
#include <sstream>

using namespace std;

namespace Socketbackend {

void UnixConnector::reconnect() {
  struct stat buf;
  int errno;
  struct sockaddr_un sa;

  if (connected) return;
  close(sock);
  sock = -1;

  //L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<options["path"]<<endl;

  // check if path exists
  if ((errno = stat(options["path"].c_str(), &buf))!=0) {
    //L<<Logger::Error<<"Cannot connect: "<<strerror(errno)<<endl;
    return;
  }

  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  sa.sun_family = AF_UNIX;
  strncpy(sa.sun_path, options["path"].c_str(), UNIX_PATH_MAX);

  if (connect(sock, reinterpret_cast<struct sockaddr*>(&sa), sizeof sa)) {
    //L<<Logger::Error<<"Cannot connect: "<<strerror(errno)<<endl;
    return;
  }

  connected = true;
}

bool UnixConnector::query(const std::string &request)
{
  unsigned char tmp[4];
  const char *str;
  reconnect();
  if (!connected) return false;
  str = request.c_str();
  // send data size first
  write(sock, str, strlen(str));
  write(sock, "\n", 1);
};

bool UnixConnector::reply(std::string &result)
{
  stringstream ss(stringstream::in);
  fd_set rs;
  struct timeval tv;
  time_t t;
  char data[1500];
  if (!connected) return false;

  FD_ZERO(&rs);
  FD_SET(sock, &rs);
  
  t = time(NULL);

  while(time(NULL) - t < timeout) {
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    if (select(sock+1, &rs, NULL, NULL, &tv)>0) {
      if (FD_ISSET(sock, &rs)) {
        size_t rlen;
        rlen = read(sock, data, sizeof data);
        if (rlen == 0) { 
           // EOF
           connected = false;
           return false;
        }
        ss << data;
        if (index(data, '\n')>0) { result = ss.str(); return true; }
      }
    }
  }

  // must have been timed out... 
  return false;
};

};
