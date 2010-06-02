#include "AiboHead.h"

AiboHead::AiboHead()
{

}

/*! Create socket for head (PTZ) and connect
 *  @param ip_addr - The ip of the Aibo
 */
void AiboHead::connect(const char *ip_addr)
{

    aibolink = new AiboNet(ip_addr, AIBO_HEAD_PORT);

}


/*! Moves the head up to a give position */
int AiboHead::up(float magnitude)
{

    aibolink->send_data('t', magnitude);
    return 0;
}

/*! Moves the head down to a give position */
int AiboHead::down(float magnitude)
{

    aibolink->send_data('t', -magnitude);
    return 0;
}

/*! Moves the head left to a give position */
int AiboHead::left(float magnitude)
{

    aibolink->send_data('p', magnitude);
    return 0;
}

/*! Moves the head right to a give position */
int AiboHead::right(float magnitude)
{

    aibolink->send_data('p', -magnitude);
    return 0;
}

/*! Moves the head to a centered position */
int AiboHead::center()
{

    aibolink->send_data('p', 0);
    return 0;
}

int AiboHead::yaw(float magnitude)
{

    aibolink->send_data('r', magnitude);
    return 0;
}

int AiboHead::pitch(float magnitude)
{

    aibolink->send_data('r', -magnitude);
    return 0;
}

/*! Designed to take arguments from Player message
 *  @param pan - The position to pan the head to
 *  @param tilt - The position to tilt the head to
 *  @param zoom - ...
 */
int AiboHead::move(float pan, float tilt, float zoom)
{

    aibolink->send_data('p', pan);
    aibolink->send_data('t', tilt);
    aibolink->send_data('r', zoom);

    return 0;
}

/*! AiboHead desconstructor
 *  Deletes the socket
 */
AiboHead::~AiboHead()
{

    delete aibolink;

}
