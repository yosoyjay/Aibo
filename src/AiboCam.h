#ifndef __AIBOCAM_H__
#define __AIBOCAM_H__

#define AIBO_CAM_PORT 10011

#include "dev.h"
#include "AiboNet.h"
#include "RWLock.h"

/**
 *  This class represents the Aibo's camera.  Note that the 
 *  initial updateMMap() and connect() calls must exists
 *  exactly where there are called in MainSetup().
 */
//! \brief The Aibo's camera.
class AiboCam : public dev
{
public:
    AiboCam();
    int updateMMap(int decompress);
    void connect(const char *ip_addr);
    RWLock lock;
    ~AiboCam();
private:
    AiboNet *aibolink;
};

#endif
