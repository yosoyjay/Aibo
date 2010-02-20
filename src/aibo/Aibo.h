#ifndef __AIBO_H__
#define __AIBO_H__

#include "dev.h"
#include "Socket.h"
#include "RWLock.h"

class AiboCam : public dev {
 public:
  AiboCam(const char *hostname, int port, int tcp);
  int updateMMap(int decompress);
  Socket *sock;
  RWLock lock;
  int tcp;
};

#endif
