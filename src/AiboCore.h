#ifndef AIBO_CORE_H
#define AIBO_CORE_H

#include <unistd.h>
#include <pthread.h>
#include <libplayercore/playercore.h>

#include "AiboWalk.h"
#include "AiboHead.h"
#include "AiboCam.h"

//! \brief The entire Aibo and required Player classes. Core of driver.
class AiboCore : public ThreadedDriver
{
public:
    AiboCore();
    AiboCore(const char *ip_addr);
    AiboCore(ConfigFile* cf, int section);
    void connect(const char *ip_addr);
    int count();
    ~AiboCore();

    AiboWalk walk;
    AiboHead head;
    AiboCam cam;

    /* Functions required for Player
     *
     */
    virtual int  MainSetup();
    virtual void MainQuit();

    int ProcessMessage( QueuePointer &resp_queue, player_msghdr* hdr, void* data );

private:
    static int aibo_count;
    //id_array_struct

    /* Functions required for Player
     *
     */
    virtual void Main();

    // Device addresses for proxies used in constructor
    player_devaddr_t position_addr;
    player_devaddr_t ptz_addr;
    player_devaddr_t camera_addr;

    // IP/Ports - Probably should be moved?
    const char* ip;
    int segCam_com_port;
    int rawCam_com_port;
    int protocol;

    // Just a pointer to images from AiboCam
    uint8_t* picture;

    // File Descriptors for socket.
    int segCam_fd;
    int rawCam_fd;

    // Mutex to stop walking from slowing down system
    pthread_mutex_t walk_mutex;

    // temporary place to hold AiboCam aibo
    //AiboCam* cam;

    // Position2d proxy variables
    player_position2d_cmd_vel_t position_cmd;
    player_ptz_cmd_t head_cmd;
};

// j - where the heck should these go?
Driver* Aibo_Init( ConfigFile* cf, int section);

void Aibo_Register( DriverTable *table);

#endif
