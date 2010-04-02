#include "AiboCore.h"

int AiboCore::aibo_count = 0;

AiboCore::AiboCore()
{
    ++AiboCore::aibo_count;
}

AiboCore::AiboCore(const char ip_addr[])
{
    walk.connect(ip_addr);
    head.connect(ip_addr);
    cam.connect(ip_addr);
    ++AiboCore::aibo_count;
}

void AiboCore::connect(const char ip_addr[])
{

    walk.connect(ip_addr);
    head.connect(ip_addr);
    cam.connect(ip_addr);

}

int AiboCore::count()
{
    return AiboCore::aibo_count;
}

AiboCore::~AiboCore()
{


}
