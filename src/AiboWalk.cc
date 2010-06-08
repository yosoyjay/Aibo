#include "AiboWalk.h"
#include "unistd.h"

AiboWalk::AiboWalk()
{

}

void AiboWalk::connect(const char *ip_addr)
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

int AiboWalk::walk(float px, float py,float pa)
{

	// Convert m/s to commands for Aibo (0.0, 0.9)	
	px /= 0.42; py /= 0.42;
	
    px > 0.8 ? px = 0.8 : px;
    py > 0.8 ? py = 0.8 : py;
    pa > 0.8 ? pa = 0.8 : pa;

	px < -0.8 ? px = -0.8 : px;
    py < -0.8 ? py = -0.8 : py;
    pa < -0.8 ? pa = -0.8 : pa;

    aibolink->send_data('f', px);
    aibolink->send_data('s', py);
    aibolink->send_data('r', pa);
    return 0;
}

AiboWalk::~AiboWalk()
{

    delete aibolink;

}
