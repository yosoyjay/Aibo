#include "AiboHead.h"

AiboHead::AiboHead()
{

}

void AiboHead::connect(const char *ip_addr)
{

    aibolink = new AiboNet(ip_addr, AIBO_HEAD_PORT);

}

int AiboHead::up(float magnitude)
{

    aibolink->send_data('t', magnitude);
    return 0;
}

int AiboHead::down(float magnitude)
{

    aibolink->send_data('t', -magnitude);
    return 0;
}

int AiboHead::left(float magnitude)
{

    aibolink->send_data('p', magnitude);
    return 0;
}

int AiboHead::right(float magnitude)
{

    aibolink->send_data('p', -magnitude);
    return 0;
}

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

AiboHead::~AiboHead()
{

    delete aibolink;

}
