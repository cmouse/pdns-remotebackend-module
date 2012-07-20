#include "connector.hh"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/un.h>
#include <cstdio>
#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace Socketbackend {

void UnixConnector::reconnect() {
  struct stat buf;
  int errno;
  struct sockaddr_un sa;

  if (connected) return;

  // enable do_reuse
  do_reuse = true;

  L<<Logger::Info<<getConnectorName()<<" is (re)connecting to "<<options["path"]<<endl;

  // check if path exists
  if ((errno = stat(options["path"].c_str(), &buf)) < 0) {
    L<<Logger::Error<<"Cannot stat: "<<strerror(errno)<<endl;
    return;
  }

  if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    L<<Logger::Error<<"Cannot create socket: "<<strerror(errno)<<endl;
    return;
  }

  sa.sun_family = AF_UNIX;
  strncpy(sa.sun_path, options["path"].c_str(), UNIX_PATH_MAX);

  if (connect(sock, reinterpret_cast<struct sockaddr*>(&sa), sizeof sa) < 0) {
    L<<Logger::Error<<"Cannot connect: "<<strerror(errno)<<endl;
    return;
  }

  connected = true;
} 

};
