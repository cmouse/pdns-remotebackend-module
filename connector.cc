#include "socketbackend.hh"
#include <boost/foreach.hpp>
#include "pdns/misc.hh"
#include <cctype>

namespace Socketbackend {

Connector::Connector() { do_reuse = false; timeout = 2; };

Connector * Connector::build(const std::string &connstr) {
      Connector *conn;
      std::vector<std::string> parts;
      std::string type;
      std::string opts;

      // connstr is of format "type:options"
      size_t pos;
      pos = connstr.find_first_of(":");
      if (pos == std::string::npos)
         throw new AhuException("Invalid connection string: malformed");

      type = connstr.substr(0, pos);
      opts = connstr.substr(pos+1);

      stringtok(parts, opts, ",");

      // the first token defines the actual backend to use
      if (type == "tcp") {
        conn = new TCPConnector();
      } else if (type == "unix") {
        conn = new UnixConnector();
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
          if (opt.find_first_not_of(" ") == std::string::npos) continue;

          pos = opt.find_first_of("=");
          if (pos == std::string::npos) {
             throw new AhuException("Invalid connection string: option format is key=value");
          }
          key = opt.substr(0,pos);
          val = opt.substr(pos+1);
          conn->options[key] = val;
          if (key == "reuse" && val == "true") {
            conn->do_reuse = true;
          }
          if (key == "timeout") {
            conn->timeout = lexical_cast<int>(val);
          }
      }
      return conn;
};


bool Connector::query(const std::string &request, std::string &result)
{
   if (query(request) == false) return false;
   return reply(result);
}

bool Connector::query(JsonNode *request, JsonNode **result) 
{
   if (query(request) == false) return false;
   return reply(result);
}

bool Connector::query(JsonNode *request)
{
   bool result;
   char *s_request = json_encode(request);
   result = query(std::string(s_request));
   delete s_request;
   return result;
}

bool Connector::reply(JsonNode **result)
{
   std::string s_result;
   if (reply(s_result) == false) return false;
   cout << "Connector::reply has " << s_result << endl;
   *result = json_decode(s_result.c_str());
   if (*result == NULL) { // deocde failed?
     return false;
   }
   return true;
}

bool Connector::query(const std::string &request)
{
  const char *str;
  reconnect();
  if (!connected) return false;
  str = request.c_str();
  // send data size first
  if ((write(sock, str, strlen(str)) < 1) || (write(sock, "\n", 1) < 1)) {
    close(sock);
    connected = false;
    return false;
  }
  return true;
};

bool Connector::reply(std::string &result)
{ 
  bool ok = false;
  fd_set rs;
  struct timeval tv;
  time_t t;
  char data[1500];

  if (!connected) return ok;

  FD_ZERO(&rs);
  FD_SET(sock, &rs);

  t = time(NULL);

  while(time(NULL) - t < timeout) {
    FD_ZERO(&rs);
    FD_SET(sock, &rs);
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    if (select(sock+1, &rs, NULL, NULL, &tv)>0) {
      if (FD_ISSET(sock, &rs)) {
        size_t rlen;
        rlen = read(sock, data, sizeof data);
        data[rlen] = 0;
        if (rlen == 0) {
           // EOF
           connected = false;
           return false;
        }
        if (::index(data, '\n') != NULL) { ok = true; };

        // trim data
        result += std::string(data);
        cout << "Result is now "  << result << endl;
        if (ok) { cout << "Breaking out" << endl; break; }
      }
    }
  }

  if (!do_reuse || !ok) {
    close(sock);
    connected = false;
  }
 
  if (ok)
    cout << "returning good" << endl;
  else
    cout << "returning bad" << endl;
  
  return ok;
};


};
