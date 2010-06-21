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
    int  count();
    void walkThread();
	void headThread();
	void camThread();
    static void* startWalkThread(void *ptr);
	static void* startHeadThread(void *ptr);
	static void* startCamThread(void *prt);
    ~AiboCore();

    AiboWalk walk;
    AiboHead head;
    AiboCam  cam;

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
    const char* proto;
    int segCam_com_port;
    int rawCam_com_port;
    int protocol;

    // Just a pointer to images from AiboCam
    uint8_t* picture;

    // File Descriptors for socket.
    int segCam_fd;
    int rawCam_fd;

    // Mutex for head/walk threads
    pthread_mutex_t walk_mutex;
	pthread_mutex_t head_mutex;
	pthread_mutex_t cam_mutex;

	// pthread type for head and walk
	pthread_t head_thread;
	pthread_t walk_thread;
	pthread_t cam_thread;

	// pthread attributes
	pthread_attr_t walk_attr;
	pthread_attr_t head_attr;
	pthread_attr_t cam_attr;	

	// state variables
	bool head_update;
	bool walking;
	bool walk_alive;
	bool head_alive;
	bool cam_alive;
	bool walk_thread_started;
	bool head_thread_started;

	// AiboNet link for estop
	AiboNet* estop;	

    // Position2d proxy variables
    player_position2d_cmd_vel_t      position_cmd;
	player_position2d_cmd_vel_t      new_pos_cmd;
    player_ptz_cmd_t                 head_cmd;
	player_ptz_cmd_t                 new_head_cmd;
    player_position2d_data_t         pos_data;
	player_position2d_set_odom_req_t odom_data;

	// Camera proxy variable
    player_camera_data_t camdata;
};

// j - where the heck should these go?
Driver* Aibo_Init( ConfigFile* cf, int section);

void Aibo_Register( DriverTable *table);

#endif
