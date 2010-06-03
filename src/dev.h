#ifndef __DEVICE_H__
#define __DEVICE_H__

#define MAXDEPTH 3

//! \brief This class just wraps the image used by AiboCam
class dev
{

public:
    dev();
    dev(int w, int h, int d, int r, int g, int b);
    dev(int w, int h, int d);
    ~dev();
    int initialize(int wi, int he, int de, int r, int g, int b);
    int *getRGB()
    {
        return rgb;
    }
    void setRGB(int r, int g, int b);
    int getWidth()
    {
        return width;
    }
    int getHeight()
    {
        return height;
    }
    int getDepth()
    {
        return depth;
    }
    unsigned char *getImage()
    {
        return image;
    }
    unsigned char getByte(int position);

protected:
    unsigned char *image;
    int width;
    int height;
    int depth;
    int rgb[3];
};

#endif
