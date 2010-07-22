#ifndef AIBO_HEAD_H
#define AIBO_HEAD_H

#include "AiboNet.h"

/**
 * Controls the Aibo's head.
 */
class AiboHead
{
public:

    AiboHead();

	// Creates socket to send commands to Aibo
    void connect(const char *ip_addr, int port);

	// Sends commands via socket (aibolink)
    int move(float pan, float tilt, float zoom);

    ~AiboHead();

private:

	// Socket object
    AiboNet *aibolink;
};

#endif
