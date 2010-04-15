#ifndef __AIBOCAM_H__
#define __AIBOCAM_H__

#define AIBO_CAM_PORT 10011

#include "dev.h"
#include "AiboNet.h"
#include "RWLock.h"

class AiboCam : public dev
{
public:
    AiboCam();
    int updateMMap(int decompress);
    void connect(const char *ip_addr);
    void UDPConnect(const char *hostname);
    RWLock lock;
    ~AiboCam();
private:
    AiboNet *aibolink;
};

#endif
