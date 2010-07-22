#include "AiboHead.h"

AiboHead::AiboHead()
{

}

void AiboHead::connect(const char *ip_addr, int port)
{

    aibolink = new AiboNet(ip_addr, port);

}

int AiboHead::move(float pan, float tilt, float zoom)
{

    aibolink->send_data('p', pan);
    aibolink->send_data('t', tilt);
    aibolink->send_data('r', zoom);

    return 0;
}

AiboHead::~AiboHead()
{

    delete aibolink;

}
