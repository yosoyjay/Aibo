#ifndef AIBO_CORE_H
#define AIBO_CORE_H

#include "AiboWalk.h"
#include "AiboHead.h"
#include "AiboCam.h"

class AiboCore
{
public:
    AiboCore();
    AiboCore(const char *ip_addr);
    void connect(const char *ip_addr);
    int count();
    ~AiboCore();

    AiboWalk walk;
    AiboHead head;
    AiboCam cam;

private:
    static int aibo_count;

};

#endif
