#ifndef AIBO_WALK_H
#define AIBO_WALK_H

#include <cmath>

#include "AiboNet.h"
#include "unistd.h"


/**
 * Control's the Aibo's movement (Walking).
 */
class AiboWalk
{
public:

    AiboWalk();

	// Creates and connects to the Aibo
    void connect(const char *ip_addr, int port);

	/* Sends 3 velocity values to the Aibo.
	 * px - velocity in the x direction (m/s)
	 * py - velocity in the y direction (m/s)
	 * pa - angular velocity           (rad/s)
	 */
    int walk(float px, float py,float pa);

    ~AiboWalk();

private:

	// Socket object
    AiboNet *aibolink;

};

#endif
