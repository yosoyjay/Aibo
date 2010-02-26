#ifndef AIBO_CORE_H
#define AIBO_CORE_H

#include "AiboWalk.h"
#include "AiboHead.h"

class AiboCore
{
public:
    AiboCore();
    AiboCore(char *ip_addr);
    void connect(char *ip_addr);
    int count();
    ~AiboCore();

    AiboWalk walk;
    AiboHead head;

private:
    static int aibo_count;
    //id_array_struct

};

#endif
