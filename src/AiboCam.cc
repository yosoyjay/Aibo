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

AiboCam::AiboCam()
{
    // get image details
    depth = 3;
    // set width, height 0 to trigger automatically:
    width = 0;
    height = 0;
    //printf("updated!\n");
    //printf("intialized!\n");
}

void AiboCam::connect(const char *hostname)
{
    aibolink = new AiboNet(hostname, AIBO_CAM_PORT);
    //updateMMap(0); // this will set height and width automatically
    //initialize(width, height, depth, 0, 1, 2); // create some space
}

int AiboCam::updateMMap(int decompress)
{
    char *header, *type, *creator, *fmt, *image_buffer;
    long format, compression, newWidth, newHeight, timeStamp, frameNum, unknown1;
    long chanWidth, chanHeight, layer, chanID, unknown2, size;
    //printf("In updateMMap 1\n");
    lock.ReadLock();
    //printf("In updateMMap 2\n");
    // get an image from socket
    // Got type=TekkotsuImage
    // Got format=0
    // Got compression=1
    // Got newWidth=104
    // Got newHeight=80
    // Got timest=121465
    // Got frameNum=3185
    //printf("receiving...\n");
    header = aibolink->read(4);  // \r\0\0\0
    printf("In updateMMap 3: '%s'\n", header);
    type = aibolink->readUntil((char)0); // "TekkotsuImage"
    printf("type: '%s'\n", type);
    format = convert(aibolink->read(4));
    printf("format: %ld\n", format);
    compression = convert(aibolink->read(4));
    printf("compression: %ld\n", compression);
    newWidth = convert(aibolink->read(4));
    printf("newWidth: %ld\n", newWidth);
    newHeight = convert(aibolink->read(4));
    printf("newHeight: %ld\n", newHeight);
    timeStamp = convert(aibolink->read(4));
    printf("timeStamp: %ld\n", timeStamp);
    frameNum = convert(aibolink->read(4));
    printf("frameNum: %ld\n", frameNum);
    unknown1 = convert(aibolink->read(4));
    printf("unknown1: %ld\n", unknown1);
    //// Got creator=FbkImage
    ////// Got chanwidth=104
    //// Got chanheight=80
    ////// Got layer=3
    //// Got chan_id=0
    ////// Got fmt=JPEGColor
    //// read JPEG: len=2547
    creator = aibolink->readUntil((char)0); // creator
    printf("creator: %s\n", creator);
    chanWidth = convert(aibolink->read(4));
    printf("chanWidth: %ld\n", chanWidth);
    chanHeight = convert(aibolink->read(4));
    printf("chanHeight: %ld\n", chanHeight);
    layer = convert(aibolink->read(4));
    printf("layer: %ld\n", layer);
    chanID = convert(aibolink->read(4));
    printf("chanID: %ld\n", chanID);
    unknown2 = convert(aibolink->read(4));
    printf("unknown2: %ld\n", unknown2);
    fmt = aibolink->readUntil((char)0); // fmt
    printf("fmt: %s\n", fmt);
    size = convert(aibolink->read(4));
    printf("size: %ld\n", size);
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
