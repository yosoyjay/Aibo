#ifndef AIBO_CORE_H
#define AIBO_CORE_H

#include "AiboNet.h"

class AiboCore
{
public:
    AiboCore();
    int connect(char ip_addr[], unsigned int aibo_port);
    ~AiboCore();

private:
    AiboNet *aibolink;
};

#endif
