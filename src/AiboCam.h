#ifndef __AIBOCAM_H__
#define __AIBOCAM_H__

#include "dev.h"
#include "AiboNet.h"
#include "RWLock.h"
#include "jpeg.h"

/**
 *  This class represents the Aibo's camera.  Note that the 
 *  initial updateMMap() and connect() calls must exists
 *  exactly where there are called in MainSetup().
 */
class AiboCam : public dev
{
public:
    AiboCam();

	// Basically gets next image from the Aibo
    int updateMMap(int decompress);

	// Creates socket and connects to the Aibo via TCP
    void connect(const char *ip_addr, int port);

	// Creates socket and connects to the Aibo via UDP 
    void connect_udp(const char *ip_addr, int port);

	// Mutex object
    RWLock lock;

	// Method to check if the cam data is being published to the cam interface
	bool getProvided() { return camProvided; };

	// Set state of whether the camera is provided or not.
	void setProvided(bool b) { camProvided = b; };

    ~AiboCam();
private:

	// Socket object
    AiboNet *aibolink;

	// State of whether or not the camera interface is being provided.
	bool camProvided;

	// State of whether or not debug messages are being printed to stdout
	bool debug;
};



#endif
