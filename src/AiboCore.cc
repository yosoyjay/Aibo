#include "AiboCore.h"
#include <cstring>

/* Called during driver initialization to create socket connections */
void AiboCore::connect(const char *ip_addr)
{
    walk.connect(ip_addr, AiboWalkPort);
    head.connect(ip_addr, AiboHeadPort);
    cam.connect(ip_addr, AiboCamPort);
    state.connect(ip_addr, AiboStatePort);
}

AiboCore::~AiboCore()
{
	delete estop;
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
AiboCore::AiboCore(ConfigFile* cf, int section) : ThreadedDriver(cf, section)
{
	
	// Read information from the config file
    ip = cf->ReadString(section, "ip", "192.168.2.155"); 
    proto = cf->ReadString(section, "protocol", "TCP");
	AiboEstopPort = cf->ReadInt(section, "estop", 10053);
    AiboCamPort = cf->ReadInt(section, "cam", 10011);
	AiboHeadPort = cf->ReadInt(section, "head", 10052);
	AiboWalkPort = cf->ReadInt(section, "walk", 10050);
	AiboStatePort = cf->ReadInt(section, "state", 10031);	

    printf("Using IP: %s \n", ip);
    printf("Protocol %s\n", proto);
	
	// Needed to add interfaces
	memset(&position_addr, 0, sizeof(player_devaddr_t));
	memset(&pos_data, 0, sizeof(player_position2d_data_t));
	memset(&ptz_addr, 0, sizeof(player_devaddr_t));
	memset(&camera_addr, 0, sizeof(player_devaddr_t));
	memset(&ir_addr, 0, sizeof(player_devaddr_t));

    //Position2d
    if (cf->ReadDeviceAddr(&position_addr, section, "provides", 
	   PLAYER_POSITION2D_CODE, -1, NULL) == 0){
        if (AddInterface(position_addr) != 0)
        {
            //PLAYER_ERROR("Error adding Position2d interface\n");
            SetError(-1);
            return;
        }
        puts("Added Position2d interface to Aibo");
	} else {
        PLAYER_WARN("Position2d interface not added");
    }

    // PTZ
    if(cf->ReadDeviceAddr(&ptz_addr,section,"provides",
	   PLAYER_PTZ_CODE,-1,NULL) == 0){
        if(AddInterface(ptz_addr) != 0)
        {
            SetError(-1);
            return;
        }
        puts("Added PTZ interface to Aibo");
	} else {
		PLAYER_WARN("PTZ interface not added");
    }

    // Camera
    if(cf->ReadDeviceAddr(&camera_addr, section, "provides", 
	   PLAYER_CAMERA_CODE, -1, NULL) == 0){
        if (AddInterface(camera_addr) != 0){
            SetError(-1);
            return;
        }
		cam.set_provided (true);
        puts("Added Camera Interface to Aibo");
    } else {
		cam.set_provided (false);
		PLAYER_WARN("Camera interface not provided");
	}

	// IR (Ranger)
	if(cf->ReadDeviceAddr(&ir_addr, section, "provides",
       PLAYER_RANGER_CODE, -1, NULL) == 0){
		if (AddInterface(ir_addr) != 0){
			SetError(-1);
			return;
		}
		puts("Added Ranger Interface to Aibo");
	} else {
		PLAYER_WARN("Ranger interface not added");
	}

    // Connect head, walk, cam sockets 
    walk.connect(ip, AiboWalkPort);
    head.connect(ip, AiboHeadPort);
	state.connect(ip, AiboStatePort);

	// Only add cam if it is in provides
	if(cam.get_provided()){
		if(strncmp(proto, "TCP", 4) == 0){
			cam.connect(ip, AiboCamPort);
		} else {
			cam.connect_udp(ip, AiboCamPort);
		}
	}
	estop = new AiboNet(ip, AiboEstopPort);
}

// Set up the device.  Return 0 if things go well, and -1 otherwise.
int AiboCore::MainSetup()
{
    puts("Aibo driver initializing.");

	if(cam.get_provided()){
		//cam = new AiboCam.connect(ip);
		cam.updateMMap(0);

		// Don't ask questions.
		cam.initialize(cam.getWidth(),cam.getHeight(),3,0,1,2);
	}
    
    // Initialize mutex and pthread stuff for walking and head
    pthread_mutex_init(&walk_mutex, NULL);
	pthread_mutex_init(&head_mutex, NULL);
	pthread_mutex_init(&cam_mutex, NULL);
	pthread_mutex_init(&state.ir_mutex, NULL);

	pthread_attr_init(&walk_attr);
	pthread_attr_init(&head_attr);
	pthread_attr_init(&cam_attr);
	pthread_attr_init(&ir_attr);

	pthread_attr_setdetachstate(&walk_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setdetachstate(&head_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setdetachstate(&cam_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setdetachstate(&ir_attr, PTHREAD_CREATE_DETACHED);

	// Initialize state variables for theads
	walk_thread_started = false;
	walk_alive = true;
	head_thread_started = false;
	head_alive = true;
	cam_alive = true;
	state.ir_alive = true;

	// Just initializes walk and head
	// This needs to be done in the memory stick as a behavior.
	estop->send_data("start\n");
	walk.walk(0.1,0,0);
	head.move(0,0,0);
	sleep(1);
	estop->send_data("stop\n");

    //StartThread();

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

	pthread_mutex_lock(&cam_mutex);
	cam_alive = false;
	pthread_mutex_unlock(&cam_mutex);

	pthread_mutex_lock(&state.ir_mutex);
	state.ir_alive = false;
	pthread_mutex_unlock(&state.ir_mutex);

	pthread_attr_destroy(&walk_attr);
	pthread_attr_destroy(&head_attr);
	pthread_attr_destroy(&cam_attr);
	pthread_attr_destroy(&ir_attr);
	pthread_mutex_destroy(&walk_mutex);
	pthread_mutex_destroy(&head_mutex);
	pthread_mutex_destroy(&cam_mutex);
	pthread_mutex_destroy(&state.ir_mutex);

    //Need to put destructors here?

    puts("Aibo driver has been shutdown");
    return;
}

// Process Messages
int AiboCore::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, 
			     void *data)
{
    // Catch position2d
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
	   PLAYER_POSITION2D_CMD_VEL, position_addr)){

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
		pthread_mutex_unlock(&walk_mutex);

        return 0;
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
		      PLAYER_POSITION2D_REQ_RESET_ODOM, position_addr)){
		pthread_mutex_lock(&walk_mutex);
		pos_data.pos.px = 0;
		pos_data.pos.py = 0;
		pos_data.pos.pa = 0;
		pthread_mutex_unlock(&walk_mutex);

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
				PLAYER_POSITION2D_REQ_RESET_ODOM);
		return 0;	
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
			  PLAYER_POSITION2D_REQ_MOTOR_POWER, position_addr)){
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

		pthread_mutex_lock(&walk_mutex);
		pos_data.pos.px = odom_data.pose.px;
		pos_data.pos.py = odom_data.pose.py;
		pos_data.pos.pa = odom_data.pose.pa;		
		pthread_mutex_unlock(&walk_mutex);

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
						PLAYER_POSITION2D_REQ_SET_ODOM);
	}else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD, 
		     PLAYER_PTZ_CMD_STATE, ptz_addr)) {
			    
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
    } else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_REQ,
			  PLAYER_RANGER_DATA_RANGE, ir_addr)){

		pthread_mutex_lock(&state.ir_mutex);
		//Note! need to add sources of range_array values
		range_array[0] = 0;
		range_array[1] = 0;
		range_array[2] = 0;
		ir_data.ranges = range_array;
		ir_data.ranges_count = 3;
		Publish(ir_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
				PLAYER_RANGER_DATA_RANGE, 
				(void *) &ir_data, sizeof(ir_data), NULL);	
		pthread_mutex_unlock(&state.ir_mutex);
		return 0;
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
			  PLAYER_RANGER_REQ_GET_CONFIG, ir_addr)){
		ir_config.min_range = 0;
		ir_config.max_range = 0;	
		ir_config.range_res = 0;
		Publish(ir_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
				PLAYER_RANGER_REQ_GET_CONFIG,
				(void *) &ir_config, sizeof(ir_config), NULL);
		return 0;
	}
    return 0;
}


// Main function for device thread
void AiboCore::Main()
{ 
	pthread_mutex_lock(&cam_mutex);
	if(cam.get_provided()){
	
		// Preparation for Camera interface.  If initialed earlier problems.
		cam.updateMMap(1);
		memset(&camdata, 0, sizeof(camdata));
		camdata.width = cam.getWidth();
		camdata.height = cam.getHeight();
		camdata.fdiv = 1;
		camdata.bpp = 24;
		camdata.image_count = camdata.width*camdata.height*3;

		pthread_mutex_unlock(&cam_mutex);

		if(pthread_create(&cam_thread, NULL, &startCamThread, this) != 0){
			printf("Error creating cam thread");
			exit(-1);
		}
	} else {
		pthread_mutex_unlock(&cam_mutex);
	}

	if(pthread_create(&ir_thread, NULL, &startIRThread, this) != 0){
		printf("Error creating ir thread");
		exit(-1);
	}

    while(true)
    {
        // Test if we are supposed to cancel.  Need for proper shutdown of the thread
        pthread_testcancel();

        // Publish stuff to interface
		pthread_mutex_lock(&walk_mutex);
		Publish(position_addr, PLAYER_MSGTYPE_DATA, 
		        PLAYER_POSITION2D_DATA_STATE,
                (void*) &pos_data, sizeof(pos_data), NULL);
		pthread_mutex_unlock(&walk_mutex);

        ProcessMessages();
        // Driver manual and experience suggest that it's advantageous to sleep
        // so that Player doesn't get bogged down with Messages while looping.
        usleep(25000);
    }

    free(camdata.image);
}

// Wrapper function to call member function walkThread from pthread
void* AiboCore::startWalkThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->walkThread();
	return 0;
}

// Wrapper function to call member function headThread from pthread
void* AiboCore::startHeadThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->headThread();
	return 0;
}

// Wrapper function to call member function camThread from pthread
void* AiboCore::startCamThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->camThread();
	return 0;
}

// Wrapper function to call member function irThread from pthread
void* AiboCore::startIRThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->irThread();
	return 0;
}

// Thread responsible for sending commands to walk
void AiboCore::walkThread(){
	bool alive = true;
	while(alive){	
		pthread_mutex_lock(&walk_mutex);

		if(walking){
			walk.walk(position_cmd.vel.px,
					  position_cmd.vel.py, 
					  position_cmd.vel.pa);
		} else{
			walk.walk(0,0,0);
		}
		
		// Time to die?
		alive = walk_alive;
		
		//Update stuff for Publishing
		pos_data.vel.px = position_cmd.vel.px;
		pos_data.vel.py = position_cmd.vel.py;
		pos_data.vel.pa = position_cmd.vel.pa;

		// d = rt
		pos_data.pos.px += position_cmd.vel.px*0.025;
		pos_data.pos.py += position_cmd.vel.py*0.025;
		pos_data.pos.pa += position_cmd.vel.pa*0.025;

		pthread_mutex_unlock(&walk_mutex);
		usleep(25000);
	}
	pthread_exit(NULL);
}

// Thread responsible for sending commands to move head
void AiboCore::headThread(){
	bool alive = true;
	while(alive){
		pthread_mutex_lock(&head_mutex);
		
		if(head_update){
			head.move(head_cmd.pan, head_cmd.tilt, head_cmd.zoom);
		}

		// Time to die?
		alive = head_alive;

		pthread_mutex_unlock(&head_mutex);
		usleep(5000);
	}
	pthread_exit(NULL);
}

// Thread responsible for sending commands to update state
void AiboCore::irThread(){
		bool alive = true;
	while(alive){
		pthread_mutex_lock(&state.ir_mutex);
		//printf("in ir thread\n");
		state.read_data();
		//alive = state.ir_alive;
		pthread_mutex_unlock(&state.ir_mutex);
		sleep(1);
	}
	pthread_exit(NULL);
}

void AiboCore::camThread(){
	bool alive = true;
	while(alive){
		pthread_mutex_lock(&cam_mutex);
		// Update Camera data returns size of image
        // 1 indicates that it decompresses the image
        cam.updateMMap(1);

        // Allocate space for image or resize allocated space
        if( camdata.image == NULL){
            camdata.image = (uint8_t *) malloc(camdata.image_count);
		} else {
            camdata.image = (uint8_t *) realloc(camdata.image, 
												camdata.image_count);
        }

        // Retrieve image out of encapsulation
        picture = cam.getImage();

        // Copy image to camera interface buffer
        memcpy(camdata.image, picture, camdata.image_count);

		// Publish
		Publish(camera_addr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
                (void*) &camdata, sizeof(camdata), NULL);

		alive = cam_alive;
		pthread_mutex_unlock(&cam_mutex);
		usleep(25000);
	}
}

/* need the extern to avoid C++ name-mangling  */
extern "C" {
    int player_driver_init(DriverTable* table){
        Aibo_Register(table);
        return(0);
	}
}
