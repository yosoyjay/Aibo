#include "AiboCore.h"

/** 
 * Counter to track number of Aibo's using driver 
 */
int AiboCore::aibo_count = 0;


/** !!!!! Test compilation without this. !!!!! 
 * Called during driver initialization to create socket connections 
 *
void AiboCore::connect(const char *ip_addr)
{
    walk.connect(ip_addr, AiboWalkPort);
    head.connect(ip_addr, AiboHeadPort);
    cam.connect(ip_addr, AiboCamPort);
	state.connect(ip_addr, AiboStatePort);
}
*/

/** 
 * Returns the number of Aibo's using the driver 
 */
int AiboCore::count()
{
    return AiboCore::aibo_count;
}

AiboCore::~AiboCore()
{
	
}

/**
 * Creates a new AiboCore object.  This is the heart
 * of the driver.  
 */
Driver* Aibo_Init(ConfigFile* cf, int section)
{
    return((Driver*)(new AiboCore(cf, section)));
}

/** 
 * Tell driver table that aibo exists
 */
void Aibo_Register(DriverTable* table)
{
    table->AddDriver("aibo", Aibo_Init);
}

AiboCore::AiboCore(ConfigFile* cf, int section) : ThreadedDriver(cf, section)
{
	gotoWalking = false;

    // Clear memory structures 
	memset(&position_addr, 0, sizeof(player_devaddr_t));
	memset(&pos_data, 0, sizeof(player_position2d_data_t));
	memset(&ptz_addr, 0, sizeof(player_devaddr_t));
	memset(&camera_addr, 0, sizeof(player_devaddr_t));
	memset(&ranger_addr, 0, sizeof(player_devaddr_t));
	memset(&ranger_data, 0, sizeof(player_ranger_data_range_t));
	memset(&ranger_config, 0, sizeof(player_ranger_config_t));
	memset(&ranger_geom, 0, sizeof(player_ranger_geom_t));

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
        //PLAYER_ERROR("Error.  Constructor failed to read Position2d data from config file\n");
        SetError(-1);
        return;
    }

    // PTZ
    if(cf->ReadDeviceAddr(&ptz_addr,section,"provides",
	   PLAYER_PTZ_CODE,-1,NULL) ==0){

        if(AddInterface(ptz_addr) != 0)
        {
            SetError(-1);
            return;
        }

        puts("Added PTZ interface to Aibo");

	} else {
        SetError(-1);
        return;
    }

    // Camera
    if(cf->ReadDeviceAddr(&camera_addr, section, "provides", 
	   PLAYER_CAMERA_CODE, -1, NULL) == 0){
        if (AddInterface(camera_addr) != 0)
        {
            SetError(-1);
            return;
        }
		cam.setProvided(true);
        puts("Added Camera Interface to Aibo");
    } else {
		cam.setProvided(false);
		PLAYER_WARN("Camera interface not provided.  Check config file.");
	}

	// Ranger
	if(cf->ReadDeviceAddr(&ranger_addr, section, "provides",
       PLAYER_RANGER_CODE, -1, NULL) == 0){
		if (AddInterface(ranger_addr) != 0){
			SetError(-1);
			return;
		}
		puts("Added Ranger interface to Aibo");
	} else {
		PLAYER_WARN("Ranger interface not added");
	}
   
   	// Read information from the config file	
    ip = cf->ReadString(section, "ip", "192.168.2.155"); 
    proto = cf->ReadString(section, "protocol", "TCP");
	AiboEstopPort = cf->ReadInt(section, "estop", 10053);
    AiboCamPort = cf->ReadInt(section, "cam", 10011);
	AiboHeadPort = cf->ReadInt(section, "head", 10052);
	AiboWalkPort = cf->ReadInt(section, "walk", 10050);
	AiboStatePort = cf->ReadInt(section, "state", 10031);	

	// Useful information when using > 1 Aibos
    printf("Using IP: %s \n", ip);
    printf("Protocol %s\n", proto);

    // Connect head, walk, cam, and estop ports 
    walk.connect(ip, AiboWalkPort);
    head.connect(ip, AiboHeadPort);
	state.connect(ip, AiboStatePort);
	estop = new AiboNet(ip, 10053);

	// Only connect cam if camera interface is made available
	if ( cam.getProvided() ) {
    	if (strncmp(proto, "TCP", 4) == 0) {
			cam.connect(ip, AiboCamPort);
	    } else {
			cam.connect_udp(ip, AiboCamPort);
   	 	}
	}
    ++AiboCore::aibo_count;
}

/** 
 * Set up the driver for each use. 
 */
int AiboCore::MainSetup()
{
    puts("Aibo driver initializing.");

	if ( cam.getProvided() ) {
    	//cam = new AiboCam.connect(ip);
	    cam.updateMMap(0);

		// Don't ask questions.
    	cam.initialize(cam.getWidth(),cam.getHeight(),3,0,1,2);
	}
    
    // Initialize mutex and pthread stuff for walking and head
    pthread_mutex_init(&walk_mutex, NULL);
	pthread_mutex_init(&head_mutex, NULL);
	pthread_mutex_init(&cam_mutex, NULL);
	pthread_mutex_init(&state_mutex, NULL);
	pthread_mutex_init(&printf_mutex, NULL);

	pthread_attr_init(&walk_attr);
	pthread_attr_init(&head_attr);
	pthread_attr_init(&cam_attr);

	pthread_attr_setdetachstate(&walk_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setdetachstate(&head_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setdetachstate(&cam_attr, PTHREAD_CREATE_DETACHED);

	// Default state values
	walk_thread_started = false;
	walk_alive = true;
	head_thread_started = false;
	head_alive = true;
	cam_alive = true;
	state_alive = true;

	// Just initializes walk and head
	// Estop is set to 'On,' alas it will not move.
	estop->send_data("start\n");
	head.move(0,1,0.5);

	// Message for checking status:
    puts("Aibo driver ready");

    return 0;
}

/** 
 * Shutdown the driver.  Basically set states and
 * destroy mutex related stuff.
 */
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

	pthread_mutex_lock(&state_mutex);
	cam_alive = false;
	pthread_mutex_unlock(&state_mutex);

	pthread_attr_destroy(&walk_attr);
	pthread_attr_destroy(&head_attr);
	pthread_attr_destroy(&cam_attr);
	pthread_attr_destroy(&state_attr);
	pthread_mutex_destroy(&walk_mutex);
	pthread_mutex_destroy(&head_mutex);
	pthread_mutex_destroy(&cam_mutex);
	pthread_mutex_destroy(&state_mutex);

    //  !!!!!!Need to put destructors here?

    puts("Aibo driver has been shutdown");
    return;
}

/** 
 * Process Messages.  This method catches messages published to the driver
 * and routes them to the appropriate Player interface.
 */
int AiboCore::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, 
			     void *data)
{
    // Catch position2d
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
	   PLAYER_POSITION2D_CMD_VEL, position_addr)){
		//printf("Request received to SetSpeed\n");
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
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, 
	   PLAYER_POSITION2D_CMD_POS, position_addr)){
		printf("Request received to GoTo\n");
        assert(hdr->size == sizeof(player_position2d_cmd_pos_t));

		pthread_mutex_lock(&walk_mutex);

		// Copy data to member struct
		if(gotoWalking == false){
			gotoWalking = true;	
        	position_goto = *(player_position2d_cmd_pos_t *) data;
			pthread_create(&goto_thread, NULL, &startGotoThread, this); 
		}

		pthread_mutex_unlock(&walk_mutex);
        return 0;

	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ, 
		      PLAYER_POSITION2D_REQ_RESET_ODOM, position_addr))
	{
		//printf("Request received to reset Odometry\n");
		pthread_mutex_lock(&walk_mutex);
		pos_data.pos.px = 0;
		pos_data.pos.py = 0;
		pos_data.pos.pa = 0;
		pthread_mutex_unlock(&walk_mutex);

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
				PLAYER_POSITION2D_REQ_RESET_ODOM);
		return 0;	
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
			  PLAYER_POSITION2D_REQ_MOTOR_POWER, position_addr))
	{
		bool powered = ((player_position2d_power_config_t*)data)->state;
		if(powered){
			printf("Setting Estop off\n");
		} else {
			printf("Setting Estop on\n");
		}

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
			  PLAYER_POSITION2D_REQ_SET_ODOM, position_addr))
	{
		//printf("Request received to SetOdometry\n");
		odom_data = *(player_position2d_set_odom_req_t *) data;

		pthread_mutex_lock(&walk_mutex);
		pos_data.pos.px = odom_data.pose.px;
		pos_data.pos.py = odom_data.pose.py;
		pos_data.pos.pa = odom_data.pose.pa;		
		pthread_mutex_unlock(&walk_mutex);

		Publish(position_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK, 
						PLAYER_POSITION2D_REQ_SET_ODOM);
	} else if(Message::MatchMessage(hdr,PLAYER_MSGTYPE_CMD, 
		     PLAYER_PTZ_CMD_STATE, ptz_addr)) {
		//printf("Request received to move head\n");
			    
        assert(hdr->size == sizeof(player_ptz_cmd_t));
        head_cmd = *(player_ptz_cmd_t *) data;

		// Create new thread once there is a message to walk
		if(!head_thread_started){
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
	// Ranger
    } else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
			  PLAYER_RANGER_REQ_GET_CONFIG, ranger_addr)){
		printf("Request received for configuration.\n");
		ranger_config.min_range = 0.05;
		ranger_config.max_range = 1.5;	
		ranger_config.range_res = 0.01;
		Publish(ranger_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
				PLAYER_RANGER_REQ_GET_CONFIG, (void *) &ranger_config);
		return 0;
	} else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_REQ,
			  PLAYER_RANGER_REQ_GET_GEOM, ranger_addr)){
		printf("Request received for geometry\n");
		//ranger_geom.element_sizes_count = 3;
		Publish(ranger_addr, resp_queue, PLAYER_MSGTYPE_RESP_ACK,
				PLAYER_RANGER_REQ_GET_GEOM, (void *) &ranger_geom);
		return 0;
	}

    return 0;
}


/** 
 * The main loop exists here.  Timer implemented to ensure that it loops
 * no faster than the Aibo can update.
 * 
 * Camera: Certain camera set-up must take place here, and no earlier.
 * State:  State thread spawned here.
 */
void AiboCore::Main()
{
		
	pthread_mutex_lock(&cam_mutex);
	if(cam.getProvided()){
		// Preparation for Camera interface.  If initialed earlier problems.
	    cam.updateMMap(1);
   		memset(&camdata, 0, sizeof(camdata));
	    camdata.width = cam.getWidth();
	   	camdata.height = cam.getHeight();
   		camdata.fdiv = 1;
	    camdata.bpp = 24;
	    camdata.format = PLAYER_CAMERA_FORMAT_RGB888;
    	camdata.compression = PLAYER_CAMERA_COMPRESS_RAW;
    	camdata.image_count = camdata.width*camdata.height*3;
		camTimer = new metrobotics::PosixTimer();
		camTimer->start();
		camTime = 0.05;	
		pthread_mutex_unlock(&cam_mutex);
		
		if(pthread_create(&cam_thread, NULL, &startCamThread, this) != 0){
			printf("Error creating cam thread");
			exit(-1);
		}		
	} else {
		pthread_mutex_unlock(&cam_mutex);
	}

			
	// State Thread
	if(pthread_create(&state_thread, NULL, &startStateThread, this) != 0){
			printf("Error creating state thread");
			exit(-1);
	}


	
	loopTimer = new metrobotics::PosixTimer();
    while(true)
    {
		loopTimer->start();
        pthread_testcancel();
		ProcessMessages();
		
        // Driver manual and experience suggest that it's advantageous to sleep
        // so that Player doesn't get bogged down with Messages while looping.
		if( loopTimer->elapsed() < 0.01 ){
			double remain = 0.01 - loopTimer->elapsed();
			usleep(remain * 1000000);
		}
    }

    free(camdata.image);
}

// Wrapper function to call member function walkThread from pthread
void* AiboCore::startWalkThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->walkThread();
	return 0;
}

// Wrapper function to call member function walkThread from pthread
void* AiboCore::startGotoThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->gotoThread();
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


// Wrapper function to call member function stateThread from pthread
void* AiboCore::startStateThread(void *ptr){
	AiboCore* bot = (AiboCore *) ptr;
	bot->stateThread();
	return 0;
}

// Thread responsible for sending commands to walk
void AiboCore::walkThread(){
	bool alive = true;
	while(alive){	
		pthread_mutex_lock(&walk_mutex);
		//printf("Updating walk thread ");

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
		pos_data.vel = position_cmd.vel;

		//Print for debug
		//printf("PosInThread: %f %f %f \n", pos_data.pos.px, pos_data.pos.py, pos_data.pos.pa);

		// d = rt
		pos_data.pos.px += position_cmd.vel.px*0.05;
		pos_data.pos.py += position_cmd.vel.py*0.05;
		pos_data.pos.pa += position_cmd.vel.pa*0.05;
		
		//Print for debug
		//printf("PosInThread: %f %f %f \n", pos_data.pos.px, pos_data.pos.py, pos_data.pos.pa);
		Publish(position_addr, PLAYER_MSGTYPE_DATA, 
		        PLAYER_POSITION2D_DATA_STATE, (void*) &pos_data);

		pthread_mutex_unlock(&walk_mutex);
		usleep(50000);
	}
	// Stop!
	walk.walk(0,0,0);
	pthread_exit(NULL);
}

// Thread responsible for sending GoTo commands to Aibo
void AiboCore::gotoThread(){
	printf("***** IN GOTO THREAD *****\n");
	printf("* Going To %f %f %f\n", position_goto.pos.px, position_goto.pos.py, position_goto.pos.pa);
	double deltaYaw = 0.0;
	bool rotateToDest = true;
	bool walkToDest = false;
	bool rotateToYaw = false;
	bool alive = false;
	gotoTimer = new metrobotics::PosixTimer();

	// If no velocities are given (This is understood to be [0.0, 0.0, 0.0]), the use 0.3 0.3
	if( position_goto.vel.px == 0.0 || position_goto.vel.pa  == 0.0 || position_goto.vel.pa == 0.0 ){
		printf("* Using default velocity (0.1, 0.3) in GoTo\n");
		position_goto.vel.px = 0.1;    position_goto.vel.pa = 0.0;     position_goto.vel.pa = 0.3;
	}

	// If only rotating, skip some steps. Using epsilon since they are floats
	if( fabs(position_goto.pos.px - pos_data.pos.px) < 0.05 && 
		fabs(position_goto.pos.py - pos_data.pos.py) < 0.05){
   		printf("* Only rotating\n");
		rotateToDest = false;
		rotateToYaw = true;		
	}

	// This should be mutexed...
	pthread_mutex_lock(&walk_mutex);
	player_pose2d_t local_data = pos_data.pos;	
	//printf("* local_data %f, %f, %f\n", local_data.px, local_data.py, local_data.pa); 
	pthread_mutex_unlock(&walk_mutex);

	pthread_mutex_lock(&goto_mutex);
	if(rotateToDest){
		printf("* Current pose: %f %f %f\n", local_data.px, local_data.py, local_data.pa);
		float gotoYaw = atan2f( position_goto.pos.py - local_data.py , 
								position_goto.pos.px - local_data.px );

		// Calculate amount to turn
		if(gotoYaw > local_data.pa){
			deltaYaw = gotoYaw - local_data.pa;
			position_goto.vel.pa = fabs(position_goto.vel.pa); 
		} else {
			if(gotoYaw <= 0 && local_data.pa <= 0){ 
				deltaYaw = fabs(gotoYaw - local_data.pa);
			} else {
				deltaYaw = fabs(gotoYaw) + local_data.pa;
			}	
			if(position_goto.vel.pa > 0)  position_goto.vel.pa *= -1;
		}
		printf("* CurrentYaw: %f, RotatingTo: %f, AmountToRotate %f, YawVel %f\n",
			    local_data.pa, gotoYaw, deltaYaw, position_goto.vel.pa);

		// Calculate time to turn
		double time = fabs(deltaYaw / position_goto.vel.pa);
		printf("* Time to turn: %f\n", time);

		// Turn at given rate for calculated time, then stop.
		alive = true;
		gotoTimer->start();
		while(alive && time != 0){
			walk.walk(0.0,0.0, position_goto.vel.pa);
			if(gotoTimer->elapsed() >= time) alive = false;
			usleep(25000);
			pthread_mutex_lock(&walk_mutex);			
			pos_data.pos.pa += position_goto.vel.pa*0.025;		
			local_data.pa += position_goto.vel.pa*0.025;
			pthread_mutex_unlock(&walk_mutex);
			//printf("* Time: %f, Elapsed %f, Remaining %f\n", time, gotoTimer->elapsed(), time - gotoTimer->elapsed());
		}
		walk.walk(0.0, 0.0, 0.0);

		// Update state
		rotateToDest = false;
		walkToDest = true;
	} 

	if(walkToDest){
		printf("* Current pose: %f %f %f\n", local_data.px, local_data.py, local_data.pa);
		// Calculate distance to walk using "distance formula"
		float diffX = position_goto.pos.px - local_data.px;
		float diffY = position_goto.pos.py - local_data.py ;	
		double distToWalk = sqrt( diffX * diffX + diffY * diffY ); 
		printf("* Walking forward %f meters dx: %f dy: %f\n", distToWalk, diffX, diffY);
	
		// Calculate the time to walk. Absolute value to avoid neg. time.
		double time = fabs(distToWalk / position_goto.vel.px);
		printf("* Time to walk: %f\n", time);
	
		// Walk for given amount of time, then stop.
		alive = true;
		gotoTimer->start();
		while(alive && time != 0){
			walk.walk(position_goto.vel.px, 0.0, 0.0);
			if(gotoTimer->elapsed() >= time) alive = false;
			usleep(25000);
			pthread_mutex_lock(&walk_mutex);			
			pos_data.pos.px += cos(position_goto.vel.px*0.025);
			pos_data.pos.py += sin(position_goto.vel.py*0.025);
			local_data.px += position_goto.vel.px*0.025;
			pthread_mutex_unlock(&walk_mutex);

			//printf("* Time: %f, Elapsed %f, Remaining %f\n", time, gotoTimer->elapsed(), time - gotoTimer->elapsed());
		}
		walk.walk(0, 0, 0);
	
		// Update state
		walkToDest = false;
		rotateToYaw = true;
	}
	
	if(rotateToYaw){
		printf("* Current pose: %f %f %f\n", local_data.px, local_data.py, local_data.pa);
		float gotoYaw = position_goto.pos.pa;
	
		// Calculate amount to turn
		if(gotoYaw > local_data.pa){
			deltaYaw = gotoYaw - local_data.pa;
			position_goto.vel.pa = fabs(position_goto.vel.pa); 
		} else {
			if(gotoYaw <= 0 && local_data.pa <= 0){ 
				deltaYaw = fabs(gotoYaw - local_data.pa);
			} else {
				deltaYaw = fabs(gotoYaw) + local_data.pa;
			}	
			if(position_goto.vel.pa > 0)  position_goto.vel.pa *= -1;
		}
		printf("* CurrentYaw: %f, RotatingTo: %f, AmountToRotate %f, YawVel %f\n",
			    local_data.pa, gotoYaw, deltaYaw, position_goto.vel.pa);
	
		// Calculate time to turn, absolute value to avoid negative time
		double time = fabs(deltaYaw / position_goto.vel.pa);
		printf("* Time to rotate: %f\n", time);	

		// Turn at given rate for calculated time, then stop.
		alive = true;
		gotoTimer->start();
		while(alive){
			walk.walk(0.0,0.0, position_goto.vel.pa);
			if(gotoTimer->elapsed() >= time) alive = false;
			usleep(25000);
			
			// Update Odometry data
			pthread_mutex_lock(&walk_mutex);			
			pos_data.pos.pa += position_goto.vel.pa*0.025;
			local_data.pa += position_goto.vel.pa*0.025;
			pthread_mutex_unlock(&walk_mutex);
			//printf("* Time: %f, Elapsed %f, Remaining %f\n", time, gotoTimer->elapsed(), time - gotoTimer->elapsed());
		}
		walk.walk(0, 0, 0);

		// Update state
		rotateToYaw = false;
	}
	pos_data.pos = position_goto.pos; 
	pthread_mutex_unlock(&goto_mutex);
	printf("****** OUT OF GOTO THREAD ******\n");
	printf("Final pose: %f %f %f\n", pos_data.pos.px, pos_data.pos.py, pos_data.pos.pa);

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
		usleep(50000);
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
            camdata.image = (uint8_t *) realloc(camdata.image, camdata.image_count);
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

		usleep(33333);
	}
	pthread_exit(NULL);
}


// Thread responsible for collecting state data
void AiboCore::stateThread(){

	bool alive = true;
	while(alive){
		pthread_mutex_lock(&state_mutex);
		range_array = new double[3];

		
		state.readData();

		// Update info for Publishing
		for(int i = 0; i < 3; i++){
			range_array[i] = state.stateData.sensors[i];
		}
		
		ranger_data.ranges = range_array;
		ranger_data.ranges_count = 3;
		

		// Time to die?
		alive = state_alive;
		//printf("InStateThread: %d %f %f %f\n", ranger_data.ranges_count, ranger_data.ranges[0], 
	   // 			ranger_data.ranges[1], ranger_data.ranges[2]);
		Publish(ranger_addr, PLAYER_MSGTYPE_DATA,
				PLAYER_RANGER_DATA_RANGE, (void*) &ranger_data);
		delete range_array;
		pthread_mutex_unlock(&state_mutex);
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
