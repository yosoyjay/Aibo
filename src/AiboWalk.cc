#include "AiboWalk.h"

AiboWalk::AiboWalk()
{

}

void AiboWalk::connect(const char *ip_addr, int port)
{

    aibolink = new AiboNet(ip_addr, port);

}

int AiboWalk::walk(float px, float py,float pa)
{
	// Convert m/s to commands for Aibo (0.0, 0.8)
	px *= 6; py *= 6; pa *= 0.35; 
	
    px > 0.8 ? px = 0.8 : px;
    py > 0.8 ? py = 0.8 : py;
    pa > 0.8 ? pa = 0.8 : pa;

	px < -0.8 ? px = -0.8 : px;
    py < -0.8 ? py = -0.8 : py;
    pa < -0.8 ? pa = -0.8 : pa;

	//printf("walk %f, %f, %f\n", px, py, pa);

    aibolink->send_data('f', px);
    aibolink->send_data('s', py);
    aibolink->send_data('r', pa);
    return 0;
}

AiboWalk::~AiboWalk()
{
    delete aibolink;
}
