#ifndef AIBO_HEAD_H
#define AIBO_HEAD_H

#include "AiboNet.h"

/*! Port used by socket to send commands */
const int AIBO_HEAD_PORT = 10052;

/*! \brief Controls the Aibo's head.
 *  The move() function is designed to work with the parameters
 *  provided by the PTZ Player interface.
 *
 *  Unlike walking commands, head commands have not been observed
 *  to have a maximum rate.
 */
class AiboHead
{
public:
    AiboHead();
	/*! Create socket for head (PTZ) and connect
	 *  @param ip_addr - The ip of the Aibo
	 */
    void connect(const char *ip_addr);
	/*! Moves the head up to a given position */
    int up(float magnitude);
	/*! Moves the head down to a given position */
    int down(float magnitude);
	/*! Moves the head left to a given position */
    int left(float magnitude);
	/*! Moves the head right to a given position */
    int right(float magnitude);
	/*! Moves the head up to the center position */
    int center();

    int pitch(float magnitude);

    int yaw(float magnitude);
	/*! Designed to take arguments from Player message
	 *  @param pan - The position to pan the head to
	 *  @param tilt - The position to tilt the head to
	 *  @param zoom - ...
	 */
    int move(float pan, float tilt, float zoom);
    ~AiboHead();

private:
    AiboNet *aibolink;

};

#endif
