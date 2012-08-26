#include "remotebackend.hh"
#include <sys/socket.h>
#include <unistd.h>
#include <boost/thread/mutex.hpp>
#include <sys/select.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <boost/foreach.hpp>

#ifndef UNIX_PATH_MAX 
#define UNIX_PATH_MAX 108
#endif

TCPConnector::TCPConnector(std::map<std::string,std::string> options) {
    this->d_url = options.find("url")->second;
}

TCPConnector::~TCPConnector() {
    this->d_c = NULL;
}

size_t tcpconnector_write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    TCPConnector *tc = reinterpret_cast<TCPConnector*>(userp);
    std::string tmp(reinterpret_cast<char *>(buffer), size*nmemb);
    tc->d_data += tmp;
    return nmemb;
}

void json_as_string(const Json::Value &input, std::string &output) {
   if (input.isString()) output = input.asString();
   else if (input.isNull()) output = "";
   else if (input.isUInt()) output = lexical_cast<std::string>(input.asUInt());
   else if (input.isInt()) output = lexical_cast<std::string>(input.asInt());
   else output = "inconvertible value";
}

int TCPConnector::send_message(const Json::Value &input) {
    int rv;
    long rcode;
    struct curl_httppost *start, *end;
    char url[4096] = {0};

    std::vector<std::string> members;
    std::string method;

    this->d_c = curl_easy_init();
    this->d_data = "";
    curl_easy_setopt(d_c, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(d_c, CURLOPT_TIMEOUT, 2);

    // prep message to send
    start = end = NULL;
    method = input.get("method", Json::Value("missing")).asString();
    snprintf(url, sizeof url, this->d_url.c_str(), method.c_str());
    strncat(url, "?", 1);

    members = input["parameters"].getMemberNames();

    BOOST_FOREACH(std::string member, members) {
      std::string value;
      json_as_string(input["parameters"][member], value);
      char *c_value = curl_easy_escape(d_c, value.c_str(), value.size());
      strncat(url, member.c_str(), member.size());
      strncat(url, "=", 1);
      strcat(url, c_value); 
      strncat(url, "&", 1);
    }

    curl_easy_setopt(d_c, CURLOPT_URL, url);
    curl_easy_setopt(d_c, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(d_c, CURLOPT_WRITEFUNCTION, &(tcpconnector_write_data));
    curl_easy_setopt(d_c, CURLOPT_WRITEDATA, this);

    // then we actually do it
    if (curl_easy_perform(d_c) == 0) {
      rv = -1;
      d_c = NULL;
    } else {
      rv = 1;
      // ensure the result was OK
      if (curl_easy_getinfo(d_c, CURLINFO_RESPONSE_CODE, &rcode) != CURLE_OK || rcode != 200) {
         rv = -1;
      } else {
         rv = this->d_data.size();
      }
    }
    curl_formfree(start);
    curl_easy_cleanup(d_c);
    return rv;
}

int TCPConnector::recv_message(Json::Value &output) {
    long rcode;
    Json::Reader r;

    if (r.parse(d_data, output) == true) {
       return 1;
    }

    d_data = ""; // cleanup
    return 0;
}
