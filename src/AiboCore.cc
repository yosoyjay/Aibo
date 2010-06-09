#include "AiboCore.h"
#include <cstring>

/* Counter to track number of Aibo's using driver */
int AiboCore::aibo_count = 0;

/* Called during driver initialization to create socket connections */
void AiboCore::connect(const char *ip_addr)
{
    walk.connect(ip_addr);
    head.connect(ip_addr);
    cam.connect(ip_addr);
}

/* Returns the number of Aibo's using the driver */
int AiboCore::count()
{
    return AiboCore::aibo_count;
}

AiboCore::~AiboCore()
{
	

}

/*  Functions required for Player
 *
 */
// Create and return new instance of driver
Driver* Aibo_Init(ConfigFile* cf, int section)
{
    return((Driver*)(new AiboCore(cf, section)));
}

// Tell driver table that aibo exists
void Aibo_Register(DriverTable* table)
{
    table->AddDriver("aibo", Aibo_Init);
}

// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
AiboCore::AiboCore(ConfigFile* cf, int section)
    : ThreadedDriver(cf, section)
{
    // Needed to add interfaces
	memset(&position_addr, 0, sizeof(player_devaddr_t));
	memset(&pos_data, 0, sizeof(player_position2d_data_t));
	memset(&ptz_addr, 0, sizeof(player_devaddr_t));
	memset(&camera_addr, 0, sizeof(player_devaddr_t));

    //Position2d
    if(cf->ReadDeviceAddr(&position_addr, section, "provides", PLAYER_POSITION2D_CODE, -1, NULL) == 0)
    {
        if(AddInterface(position_addr) != 0)
        {
            //PLAYER_ERROR("Error adding Position2d interface\n");
            SetError(-1);
            return;
        }

        puts("Added Position2d interface to Aibo");
	} else
    {
        //PLAYER_ERROR("Error.  Constructor failed to read Position2d data from config file\n");
        SetError(-1);
        return;
    }

    // PTZ
    if(cf->ReadDeviceAddr(&ptz_addr,section,"provides",PLAYER_PTZ_CODE,-1,NULL) ==0)
    {
        if(AddInterface(ptz_addr) != 0)
        {
            //PLAYER_ERROR("Error adding PTZ interface\n");
            SetError(-1);
            return;
        }

        puts("Added PTZ interface to Aibo");
	} else
    {
        //PLAYER_ERROR("Error.  Constructor failed to read PTZ data from config file\n");
        SetError(-1);
        return;
    }

    // Camera
    if(cf->ReadDeviceAddr(&camera_addr, section, "provides", PLAYER_CAMERA_CODE, -1, NULL) == 0)
    {
        if (AddInterface(camera_addr) != 0)
        {
            //PLAYER_ERROR("Could not add Camera interface for Aibo");
            SetError(-1);
            return;
        }
        puts("Added Camera Interface to Aibo");
    }
    
    ip = cf->ReadString(section, "ip", "192.168.2.155"); 	// 155 is default
    proto =cf->ReadString(section, "protocol", "TCP");

    printf("Using IP: %s \n", ip);
    printf("Protocol %s\n", proto);
    // Create head, walk, cam objects
    walk.connect(ip);
    head.connect(ip);

    if(strncmp(proto, "TCP", 4) == 0){
		cam.connect(ip);
    } else{
		cam.connect_udp(ip);
    }

	estop = new AiboNet(ip, 10053);
    ++AiboCore::aibo_count;
    rawCam_com_port = cf->ReadInt(section, "rawCamPort", 10011);  	// Seg Cam Port 10012, raw 10011
}

// Set up the device.  Return 0 if things go well, and -1 otherwise.
int AiboCore::MainSetup()
{
    puts("Aibo driver initializing.");

    //cam = new AiboCam.connect(ip);
    cam.updateMMap(0);

    // Don't ask questions.
    cam.initialize(cam.getWidth(),cam.getHeight(),3,0,1,2);
    sleep(1); // JP added this on 01/13/2010

    
    // Initialize mutex for walking
    pthread_mutex_init(&walk_mutex, NULL);
	pthread_mutex_init(&head_mutex, NULL);

	pthread_attr_init(&walk_attr);
	pthread_attr_init(&head_attr);

	pthread_attr_setdetachstate(&walk_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setdetachstate(&head_attr, PTHREAD_CREATE_DETACHED);

	walk_thread_started = false;
	walk_alive = true;
	head_thread_started = false;
	head_alive = true;

    // Starts the main device thread.  Creates a new thread and executes
    // Aibo::Main() which contains the main loop for the driver.
    StartThread();

	// Message for checking status:
    puts("Aibo driver ready");

    return 0;
}

// Shutdown the device
void AiboCore::MainQuit()
{
    puts("Shutting Aibo driver down");

	pthread_mutex_lock(&walk_mutex);
	walk_alive = false;
	pthread_mutex_unlock(&walk_mutex);

	pthread_mutex_lock(&head_mutex);
	head_alive = false;
	pthread_mutex_unlock(&head_mutex);

	pthread_attr_destroy(&walk_attr);
	pthread_attr_destroy(&head_attr);
	pthread_mutex_destroy(&walk_mutex);
	pthread_mutex_destroy(&head_mutex);

    StopThread();
    //Need to put destructors here?

    puts("Aibo driver has been shutdown");
    return;
}

// Process Messages
int AiboCore::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, 
			     void *data)
{
    // Catch position2d
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL,
						     position_addr))
    {
        assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
        new_pos_cmd = *(player_position2d_cmd_vel_t *) data;

		// Create new thread once there is a message to walk
		if(!walk_thread_started){
			if(pthread_create(&walk_thread, NULL, &startWalkThread, this) != 0){
				printf("Error creating walk thread");
				exit(-1);
			}
			walk_thread_started = true;
		}		

		pthread_mutex_lock(&walk_mutex);
		position_cmd = new_pos_cmd;
		if(position_cmd.vel.px == 0 && 
		   position_cmd.vel.py == 0 && 
		   position_cmd.vel.pa == 0){
			walking = false;
		} else{
			walking = true;
		}
		printf("walking in message matching:%d\n", walking);
		pthread_mutex_unlock(&walk_mutex);

        return 0;
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
								  PLAYER_POSITION2D_REQ_RESET_ODOM, position_addr))
	{
		printf("Received request to reset odometry");

		pos_data.pos.px = 0;
		pos_data.pos.py = 0;
		pos_data.pos.pa = 0;	

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
				PLAYER_POSITION2D_REQ_RESET_ODOM);
		return 0;	
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
								  PLAYER_POSITION2D_REQ_MOTOR_POWER, position_addr))
	{
		bool powered = ((player_position2d_power_config_t*)data)->state;

		printf("Setting estop to %d\n", powered);

		if(powered){
			if(estop->send_data("start\n") < 0){
				Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
						PLAYER_POSITION2D_REQ_MOTOR_POWER);
			} 
		} else {
			if(estop->send_data("stop\n") < 0){
				Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_NACK, 
						PLAYER_POSITION2D_REQ_MOTOR_POWER);
			}
		}

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
				PLAYER_POSITION2D_REQ_MOTOR_POWER);
	  	return 0;	
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
								  PLAYER_POSITION2D_REQ_SET_ODOM,position_addr))
	{
		odom_data = *(player_position2d_set_odom_req_t *) data;
		pos_data.pos.px = odom_data.pose.px;
		pos_data.pos.py = odom_data.pose.py;
		pos_data.pos.pa = odom_data.pose.pa;		

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
						PLAYER_POSITION2D_REQ_SET_ODOM);
	}else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE,
								  ptz_addr))
    {
        assert(hdr->size == sizeof(player_ptz_cmd_t));
        head_cmd = *(player_ptz_cmd_t *) data;

		// Create new thread once there is a message to walk
		if(!walk_thread_started){
			if(pthread_create(&head_thread, NULL, &startHeadThread, this) != 0){
				printf("Error creating walk thread");
				exit(-1);
			}
			head_thread_started = true;
		}
		
		pthread_mutex_lock(&head_mutex);
        head.move(head_cmd.pan, head_cmd.tilt, head_cmd.zoom);
		pthread_mutex_unlock(&head_mutex);
        return 0;
    }

    return 0;
}

// Main function for device thread
void AiboCore::Main()
{
    // Preparation for Camera interface
    cam.updateMMap(1);

    player_camera_data_t camdata;
    memset(&camdata, 0, sizeof(camdata));

    camdata.width = cam.getWidth();
    camdata.height = cam.getHeight();
    camdata.fdiv = 1;
    camdata.bpp = 24;
    camdata.format = PLAYER_CAMERA_FORMAT_RGB888;
    camdata.compression = PLAYER_CAMERA_COMPRESS_RAW;
    camdata.image_count = camdata.width*camdata.height*3;

	int picSize;

	// Just initializes walk and head
	walk.walk(0,0,0);
	head.move(0,0,0);

    while(true)
    {
        // Test if we are supposed to cancel.  Need for proper shutdown of the thread
        pthread_testcancel();

        //  This is used to publish position2d data
		pthread_mutex_lock(&walk_mutex);
		pos_data.vel.px = position_cmd.vel.px;
		pos_data.vel.py = position_cmd.vel.py;
		pos_data.vel.pa = position_cmd.vel.pa;
		pthread_mutex_unlock(&walk_mutex);

		pos_data.pos.px += position_cmd.vel.px*0.025;
		pos_data.pos.py += position_cmd.vel.py*0.025;
		pos_data.pos.pa += position_cmd.vel.pa*0.025;

        Publish(position_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
                (void*) &pos_data, sizeof(pos_data), NULL);

        // Update Camera data returns size of image
        // 1 indicates that it decompresses the image
        picSize = cam.updateMMap(1);

        // Allocate space for image or resize allocated space
        if( camdata.image == NULL)
        {
            camdata.image = (uint8_t *) malloc(camdata.image_count);
        }
        else
        {
            camdata.image = (uint8_t *) realloc(camdata.image, camdata.image_count);
        }

        // Retrieve image out of encapsulation
        picture = cam.getImage();

        // Copy image to camera interface buffer
        memcpy(camdata.image, picture, camdata.image_count);

        // Publish camera stuff to interface
        Publish(camera_addr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
                (void*) &camdata, sizeof(camdata), NULL);
        ProcessMessages();

        // Driver manual and experience suggest that it's advantageous to sleep
        // so that Player doesn't get bogged down with Messages in the while loop.
        usleep(25000);
    }

    free(camdata.image);
}

void* AiboCore::startWalkThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->walkThread();
	return 0;
}

void* AiboCore::startHeadThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->headThread();
	return 0;
}

void AiboCore::walkThread(){
	bool alive = true;
	while(alive){	
		printf("in walk thread\n");
		
		pthread_mutex_lock(&walk_mutex);
		printf("walking in thread %d\n", walking);
		if(walking){
			printf("walking command sent\n");
			walk.walk(position_cmd.vel.px,
					  position_cmd.vel.py, 
					  position_cmd.vel.pa);
		} else{
			printf("not walking command\n");
			walk.walk(0,0,0);
		}
		alive = walk_alive;
		printf("walk alive %d\n", alive);
		pthread_mutex_unlock(&walk_mutex);
		usleep(25000);
	}
	pthread_exit(NULL);
}

void AiboCore::headThread(){
	bool alive = true;
	while(alive){
		pthread_mutex_lock(&head_mutex);
		if(head_update){
			head.move(head_cmd.pan, head_cmd.tilt, head_cmd.zoom);
		}
		alive = head_alive;
		pthread_mutex_unlock(&head_mutex);
		usleep(25000);
	}
	pthread_exit(NULL);
}

/* need the extern to avoid C++ name-mangling  */
extern "C" {
    int player_driver_init(DriverTable* table)
    {
        Aibo_Register(table);
        return(0);
    }
}

