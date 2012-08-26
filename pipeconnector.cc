#include "remotebackend.hh"

PipeConnector::PipeConnector(std::map<std::string,std::string> options) {
  this->command = options.find("command")->second;
  this->options = options;
  this->coproc = NULL;
  launch();
}

PipeConnector::~PipeConnector(){
  if (this->coproc != NULL) 
    delete coproc; 
}

void PipeConnector::launch() {
  if (coproc != NULL) return;

  Json::Value init,res;
  coproc = new CoProcess(this->command, 2);
  init["method"] = "initialize";
  init["parameters"] = Json::Value();

  for(std::map<std::string,std::string>::iterator i = options.begin(); i != options.end(); i++)
    init["parameters"][i->first] = i->second;

  this->send(init);
  if (this->recv(res)==false) {
    L<<Logger::Error<<"Failed to initialize coprocess"<<std::endl;
  }
}

int PipeConnector::send_message(const Json::Value &input)
{
   std::string data;
   Json::FastWriter writer;
   data = writer.write(input);

   launch();
   try {
      coproc->send(data);
      return 1;
   }
   catch(AhuException &ae) {
      delete coproc;
      coproc=NULL;
      throw;
   } 
}

int PipeConnector::recv_message(Json::Value &output) 
{
   Json::Reader r;
   launch();
   try {
      std::string line;
      coproc->receive(line); // hope it is in one line...
      if (r.parse(line,output) == true) 
        return 1;
      return -1; // wasn't json
   }
   catch(AhuException &ae) {
      L<<Logger::Warning<<"[pipeconnector] "<<" unable to receive data from coprocess. "<<ae.reason<<endl;
      delete coproc;
      coproc = NULL;
      throw;
   }
}
