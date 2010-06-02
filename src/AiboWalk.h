#ifndef AIBO_WALK_H
#define AIBO_WALK_H

#include <cmath>
#include "AiboNet.h"

/*! Port used by socket to send walk commands to the Aibo */
const int AIBO_WALK_PORT = 10050;

/*! \brief Control's the Aibo's movement (Walking).
 *  The walk() function is designed to be used with the parameters
 *  provided by the Position2d interface.  
 * 
 *  The Aibo can receive walk commands at a maximum rate of once every
 *  0.25 seconds.  If this rate is exceeded the commands will not
 *  be excepeted an result in a "Mech Command Error" (or something like that)
 *  on the Aibo. 
 */
class AiboWalk
{
public:
    AiboWalk();
    /*! Creates socket used for walking commands and connects
    to the Aibo */
    void connect(const char *ip_addr);
    /*! Sends a command for the Aibo to walk forward
 	* @param magnitude - Velocity of walk (0 - 0.9)
    */
    int forward(float magnitude);
	/*! Sends a command for the Aibo to walk backward
 	* @param magnitude - Velocity of walk (0 - 0.9)
 	*/
    int backward(float magnitude);
	/*! Sends a command for the Aibo to strafe left
	 * @param magnitude - Velocity of strafe (0 - 0.9)
 	 */
    int strafe_right(float magnitude);
	/*! Sends a command for the Aibo to strafe right
	 * @param magnitude - Velocity of strafe (0 - 0.9)
	 */
    int strafe_left(float magnitude);
	/*! Sends a command for the Aibo to rotate counter-clockwise
	 * @param magnitude - Velocity of rotation (0 - 0.9)
	 */
    int rotate_clockwise(float magnitude);
	/*! Sends a command for the Aibo to rotate clockwise
	 * @param magnitude - Velocity of rotation (0 - 0.9)
	 */
    int rotate_counter_clockwise(float magnitude);
	/*! Sends command for the Aibo to move/walk.
	 * Designed to take the three parameters from Position2D.
	 *
	 * All velocities are checked to be at most 0.9.  Velocities
	 * greater than this cause the robot to move awakwardly and
	 * may result in damage to the robot.
	 *
	 * The rotation velocity is multiplied by (1/5) so that it
	 * doesn't rotate so quickly.  This actually should be part
	 * of a log function. Consider it a to do item.
	 *
	 * @param px - Velocity of movement in x direction
	 * @param py - Velocity of movement in y direction
	 * @param pa - Velocity of rotation
	 */
    int walk(float px, float py,float pa);

    ~AiboWalk();

private:
    AiboNet *aibolink;

};

#endif
