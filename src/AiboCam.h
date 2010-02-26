#ifndef AIBO_CAM_H
#define AIBO_CAM_H

#include "AiboNet.h"

const int AIBO_CAM_PORT = 10011;

struct image_packet
{
    char header[4];
    char image_id[14];
    int format;
    int compression;
    int width;
    int height;
    int timestamp;
    int framenumber;
    int unknown; // Java doesn't have unknown -- might be pixelsize 1?
    char creator[9]; //fbkimage
    int chan_width;
    int chan_height;
    int layer;
    int chan_id;
    int unknown2; //fmt in Java "blank", "Raw Image", etc
};

class AiboCam
{
public:
    AiboCam();
    void connect(char ip_addr[]);
    int recieve_image();
    ~AiboCam();

private:
    AiboNet *aibolink;

};

#endif
