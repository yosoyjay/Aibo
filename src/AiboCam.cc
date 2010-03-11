#include "AiboCam.h"

AiboCam::AiboCam()
{

}

void AiboCam::connect(char ip_addr[])
{

    aibolink = new AiboNet(ip_addr, AIBO_CAM_PORT);

}

char *AiboCam::recieve_image()
{

  aibolink->read(6);

}

AiboCam::~AiboCam()
{
    delete aibolink;
}
