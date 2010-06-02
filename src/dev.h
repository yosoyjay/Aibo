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
	/*! This function actually allocates space for image for the camera
	 *  @param wi - Image width, retrieved from header
	 *  @param he - Image height, retrieved from header
	 *  @param r  - Red value
	 *  @param g  - Green value
	 *  @param b  - Blue value
	 */
    int initialize(int wi, int he, int de, int r, int g, int b);
    int *getRGB()
    {
        return rgb;
    }
	/*!  Sets the RGB of the image*/
    void setRGB(int r, int g, int b);
	/*!  Returns the width of the allocated image buffer
	 */
    int getWidth()
    {
        return width;
    }
	/*!  Returns teh height of the allocated image buffer
	 */
    int getHeight()
    {
        return height;
    }
	/*!  Returns the depth of the allocated image buffer
	 *   No reason it should not be 3.
	 */
    int getDepth()
    {
        return depth;
    }
	/*! Returns the image buffer.
	 */
    unsigned char *getImage()
    {
        return image;
    }
	/*! Returns the specific byte given the position 
	 *  of the byte in the image array.
	 *  Notice that it is not 2d.
	 */
    unsigned char getByte(int position);

protected:
    unsigned char *image;
    int width;
    int height;
    int depth;
    int rgb[3];
};

#endif
