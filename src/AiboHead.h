#ifndef AIBO_HEAD_H
#define AIBO_HEAD_H

#include "AiboNet.h"

const int AIBO_HEAD_PORT = 10052;

class AiboHead
{
public:
    AiboHead();
    void connect(const char *ip_addr);
    int up(float magnitude);
    int down(float magnitude);
    int left(float magnitude);
    int right(float magnitude);
    int pitch(float magnitude);
    int yaw(float magnitude);
    ~AiboHead();

private:
    AiboNet *aibolink;

};

#endif
