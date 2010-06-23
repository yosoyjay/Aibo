#ifndef __AIBOCAM_H__
#define __AIBOCAM_H__

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
    void connect(const char *ip_addr, int port);
    void connect_udp(const char *ip_addr, int port);
    RWLock lock;
	bool get_provided() { return cam_provided; }
	void set_provided(bool b) { cam_provided = b; };
    ~AiboCam();
private:
    AiboNet *aibolink;
	bool cam_provided;
};

#endif
