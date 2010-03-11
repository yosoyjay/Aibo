#include "AiboWalk.h"
#include "unistd.h"

AiboWalk::AiboWalk()
{

}

void AiboWalk::connect(const char* ip_addr)
{

    aibolink = new AiboNet(ip_addr, AIBO_WALK_PORT);

}

int AiboWalk::forward(float magnitude)
{

    aibolink->send_data('f', magnitude);
    usleep(500000);
    return 0;
}

int AiboWalk::backward(float magnitude)
{

    aibolink->send_data('f', -magnitude);
    usleep(500000);
    return 0;
}

int AiboWalk::strafe_left(float magnitude)
{

    aibolink->send_data('s', magnitude);
    usleep(500000);
    return 0;
}

int AiboWalk::strafe_right(float magnitude)
{

    aibolink->send_data('s', -magnitude);
    usleep(500000);
    return 0;
}

int AiboWalk::rotate_clockwise(float magnitude)
{

    aibolink->send_data('r', magnitude);
    usleep(500000);
    return 0;
}

int AiboWalk::rotate_counter_clockwise(float magnitude)
{

    aibolink->send_data('r', -magnitude);
    usleep(500000);
    return 0;
}

AiboWalk::~AiboWalk()
{

    delete aibolink;

}
