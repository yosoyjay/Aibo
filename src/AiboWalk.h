#ifndef AIBO_WALK_H
#define AIBO_WALK_H

#include "AiboNet.h"

const int AIBO_WALK_PORT = 10050;

class AiboWalk
{
public:
    AiboWalk();
    void connect(const char* ip_addr);
    int forward(float magnitude);
    int backward(float magnitude);
    int strafe_right(float magnitude);
    int strafe_left(float magnitude);
    int rotate_clockwise(float magnitude);
    int rotate_counter_clockwise(float magnitude);
    ~AiboWalk();

private:
    AiboNet *aibolink;

};

#endif
