#ifndef AIBO_WALK_H
#define AIBO_WALK_H

#include <cmath>
#include "AiboNet.h"

//! \brief Control's the Aibo's movement (Walking).
class AiboWalk
{
public:
    AiboWalk();
    void connect(const char *ip_addr, int port);
    int forward(float magnitude);
    int backward(float magnitude);
    int strafe_right(float magnitude);
    int strafe_left(float magnitude);
    int rotate_clockwise(float magnitude);
    int rotate_counter_clockwise(float magnitude);
    int walk(float px, float py,float pa);
    ~AiboWalk();

private:
    AiboNet *aibolink;

};

#endif
