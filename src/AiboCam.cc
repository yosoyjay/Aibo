#include "AiboCam.h"

AiboCam::AiboCam()
{

}

void AiboCam::connect(char ip_addr[])
{

    aibolink = new AiboNet(ip_addr, AIBO_CAM_PORT);

}

int AiboCam::recieve_image()
{

    return sizeof(image_packet);

}

AiboCam::~AiboCam()
{
    delete aibolink;
}
