#include "AiboCore.h"

AiboCore::AiboCore(){

}

int AiboCore::connect(char ip_addr[], unsigned int aibo_port){
  aibolink = new AiboNet(ip_addr, aibo_port);

  return 0;
}

AiboCore::~AiboCore(){

  delete aibolink;

}
