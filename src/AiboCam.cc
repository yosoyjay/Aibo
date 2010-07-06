#include "AiboCam.h"
#include "jpeg.h"

#define MAXBUFSIZE 10000


long convert(char *buff)
{
    long retval = 0;
    retval += (buff[0] & 0xFF) <<  0;
    retval += (buff[1] & 0xFF) <<  8;
    retval += (buff[2] & 0xFF) << 16;
    retval += (buff[3] & 0xFF) << 24;
    return retval;
}
/*! \brief  Aibo's Camera
 *This clas represents and implements methods to capture images from the Aibo. */
AiboCam::AiboCam()
{
	debug = false;
    // get image details
    depth = 3;
    // set width, height 0 to trigger automatically:
    width = 0;
    height = 0;
    if(debug)printf("updated!\n");
    if(debug)printf("intialized!\n");
}

/** Creates and connects a socket to capture images from the Aibo */
void AiboCam::connect(const char *hostname, int port)
{
    aibolink = new AiboNet(hostname, port);
}

// JP: Added this
void AiboCam::connect_udp(const char *hostname, int port)
{
    aibolink = new AiboNet(hostname, port, UDP_PROTO);
}

/** Captures images from socket.  Argument is used to toggle 
    decompression of image.  
    1 = decompress
    0 = no-decompress */
int AiboCam::updateMMap(int decompress=1)
{
    char *header, *type, *creator, *fmt, *image_buffer, *readBuffer;
    long format, compression, newWidth, newHeight, timeStamp, frameNum, unknown1;
    long chanWidth, chanHeight, layer, chanID, unknown2, size;
    if(debug)printf("In updateMMap 1\n");
    lock.ReadLock();
	readBuffer = new char[10000];
    if(debug)printf("In updateMMap 2\n");
    // get an image from socket
    // Got type=TekkotsuImage
    // Got format=0
    // Got compression=1
    // Got newWidth=104
    // Got newHeight=80
    // Got timest=121465
    // Got frameNum=3185
    if(debug)printf("receiving...\n");
    header = aibolink->readState(4, readBuffer);  // \r\0\0\0
    if(debug)printf("In updateMMap 3: '%s'\n", header);
    type = aibolink->readUntil((char)0); // "TekkotsuImage"
    if(debug)printf("type: '%s'\n", type);
    format = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("format: %ld\n", format);
    compression = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("compression: %ld\n", compression);
    newWidth = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("newWidth: %ld\n", newWidth);
    newHeight = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("newHeight: %ld\n", newHeight);
    timeStamp = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("timeStamp: %ld\n", timeStamp);
    frameNum = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("frameNum: %ld\n", frameNum);
    unknown1 = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("unknown1: %ld\n", unknown1);
    //// Got creator=FbkImage
    ////// Got chanwidth=104
    //// Got chanheight=80
    ////// Got layer=3
    //// Got chan_id=0
    ////// Got fmt=JPEGColor
    //// read JPEG: len=2547
    creator = aibolink->readUntil((char)0); // creator
    if(debug)printf("creator: %s\n", creator);
    chanWidth = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("chanWidth: %ld\n", chanWidth);
    chanHeight = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("chanHeight: %ld\n", chanHeight);
    layer = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("layer: %ld\n", layer);
    chanID = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("chanID: %ld\n", chanID);
    unknown2 = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("unknown2: %ld\n", unknown2);
    fmt = aibolink->readUntil((char)0); // fmt
    if(debug)printf("fmt: %s\n", fmt);
    size = convert(aibolink->readState(4, readBuffer));
    if(debug)printf("size: %ld\n", size);
    image_buffer = aibolink->read(size);

    //// convert image from JPEG to RGB in mmap
    if (width == 0 && height == 0)
    {
        width = newWidth;
        height = newHeight;
        printf("New Aibo image size: %d x %d; %ld\n", width, height, size);
        lock.ReadUnlock();
        return 0;
    }

    if (decompress)
    {
        if (size > 0 && size < 10000)
        {
            //printf("decompressing...\n");
            jpeg_decompressPyro((unsigned char *)image, (width * height * depth),
                                (unsigned char *)image_buffer, (int) size);
            //printf("done!\n");
            lock.ReadUnlock();
            return size;
        }
        else
        {
            printf("Aibo camera bad JPEG size: %ld\n", size);
            lock.ReadUnlock();
            return 0;
        }
    }

    lock.ReadUnlock();
    return 0;
}

AiboCam::~AiboCam()
{
    // Need to do any freeing?
    delete aibolink;
}
