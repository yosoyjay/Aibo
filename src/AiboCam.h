#ifndef __AIBOCAM_H__
#define __AIBOCAM_H__

#define AIBO_CAM_PORT 10011

#include "dev.h"
#include "AiboNet.h"
#include "RWLock.h"

/*! \brief  Aibo's Camera
 * This class represents and implements methods to capture images from the Aibo.
 * Image details are initialized here:
 * depth = 3 - Does not change
 * width = 0 - Automatically resized when initialized() called
 * height = 0 - Automatically rezied when inttialized() called
 */
class AiboCam : public dev
{
public:

    AiboCam();
	/*! Captures images from socket.  Argument is used to toggle
	 *  decompression of image.
	 *  @param decompress - jpeg decompression option
	 *  1 = decompress
	 *  0 = no-decompress
	 */
    int updateMMap(int decompress);
	/*! Creates and connects a socket to capture images from the Aibo
 	 * @param *hostname - Pointer to the ip address of the Aibo
	 */
    void connect(const char *ip_addr);
	/*! RWLock used to prevent multiple reads of image buffer*/
    void connect_udp(const char *ip_addr);
    RWLock lock;
	/*! Deconstructor for AiboCam.
	 *  Deletes aibolink (socket)
	 */
    ~AiboCam();
private:
    AiboNet *aibolink;
};

#endif
