#ifndef __AIBO_H__
#define __AIBO_H__

#include <sys/socket.h> 
#include "dev.h"
#include "Socket.h"
#include "RWLock.h"

class AiboCam : public dev {
 public:
  AiboCam(const char *hostname, int port, int tcp);
  ~AiboCam();
  int updateMMap(int decompress);
  CamSocket *sock;
  RWLock lock;
  int tcp;
};

#endif
