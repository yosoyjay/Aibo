#include "AiboWalk.h"
#include "unistd.h"

AiboWalk::AiboWalk()
{

}

/*! Creates socket used for walking commands and connects 
    to the Aibo */
void AiboWalk::connect(const char *ip_addr)
{

    aibolink = new AiboNet(ip_addr, AIBO_WALK_PORT);

}

/*! Sends a command for the Aibo to walk forward
 * @param magnitude - Velocity of walk (0 - 0.9)
 */
int AiboWalk::forward(float magnitude)
{

    aibolink->send_data('f', magnitude);
    usleep(500000);
    return 0;
}


/*! Sends a command for the Aibo to walk backward
 * @param magnitude - Velocity of walk (0 - 0.9)
 */

int AiboWalk::backward(float magnitude)
{

    aibolink->send_data('f', -magnitude);
    usleep(500000);
    return 0;
}

/*! Sends a command for the Aibo to strafe left 
 * @param magnitude - Velocity of strafe (0 - 0.9)
 */

int AiboWalk::strafe_left(float magnitude)
{

    aibolink->send_data('s', magnitude);
    usleep(500000);
    return 0;
}

/*! Sends a command for the Aibo to strafe right
 * @param magnitude - Velocity of strafe (0 - 0.9)
 */
int AiboWalk::strafe_right(float magnitude)
{

    aibolink->send_data('s', -magnitude);
    usleep(500000);
    return 0;
}

/*! Sends a command for the Aibo to rotate counter-clockwise 
 * @param magnitude - Velocity of rotation (0 - 0.9)
 */

int AiboWalk::rotate_clockwise(float magnitude)
{

    aibolink->send_data('r', magnitude);
    usleep(500000);
    return 0;
}

/*! Sends a command for the Aibo to rotate clockwise 
 * @param magnitude - Velocity of rotation (0 - 0.9)
 */
int AiboWalk::rotate_counter_clockwise(float magnitude)
{

    aibolink->send_data('r', -magnitude);
    usleep(500000);
    return 0;
}

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
int AiboWalk::walk(float px, float py,float pa)
{

    // Cut rotation velocity to 1/5 so it doesn't rotate so fast
    // If I perform this operation in the function it dies.
    pa *= 0.2;

    px > 0.9 ? px = 0.9 : px;
    py > 0.9 ? py = 0.9 : py;
    pa > 0.9 ? pa = 0.9 : pa;

    aibolink->send_data('f', px);
    aibolink->send_data('s', py);
    aibolink->send_data('r', pa);
    return 0;
}

AiboWalk::~AiboWalk()
{

    delete aibolink;

}
