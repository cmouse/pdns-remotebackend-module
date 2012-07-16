#include "socketbackend.hh"
#include <boost/foreach.hpp>

namespace Socketbackend {

Connector * Connector::build(const std::string &connstr) {
      Connector *conn;
      std::vector<std::string> parts;
      std::string type;
      std::string opts;

      // connstr is of format "type:options"
      size_t pos;
      pos = connstr.find_first_of(":");
      if (pos == string::npos)
         throw new AhuException("Invalid connection string: malformed");

      type = connstr.substr(0, pos);
      opts = connstr.substr(pos+1);

      stringtok(parts, opts, ",");

      // the first token defines the actual backend to use
      if (parts[0].compare("unix") == 0) {
        conn = new UnixConnector();
      } else if (parts[0].compare("tcp") == 0) {
        conn = new TCPConnector();
      } else if (parts[0].compare("udp") == 0) {
        conn = new UDPConnector();
/*      } else if (parts[0].compare("pipe") == 0) {
          conn = new PipeConnector();
        }*/
      } else {
        throw new AhuException("Invalid connection string: unknown connector");
      }
      conn->connstr = connstr;

      // find out some options and parse them while we're at it
      BOOST_FOREACH(std::string opt, parts) {
          std::string key,val;
          if (opt.find_first_not_of(" ") == string::npos) continue;

          pos = opt.find_first_of("=");
          if (pos == string::npos) {
             throw new AhuException("Invalid connection string: option format is key=value");
          }
          key = opt.substr(0,pos);
          val = opt.substr(pos+1);
          conn->options[key] = val;
          if (key == "reuse" && val == "true") {
            conn->do_reuse = true;
          }
          if (key == "http" && val == "true") {
            conn->do_http = true;
          }
          if (key == "timeout") {
            conn->timeout = lexical_cast<int>(val);
          }
      }
      return conn;
};

};
