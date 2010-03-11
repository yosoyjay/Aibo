#include "AiboCore.h"

int AiboCore::aibo_count = 0;

/*  Appears not work with Player's ThreadedDriver
AiboCore::AiboCore() : ThreadedDriver(cf, section)
{
    ++AiboCore::aibo_count;
}

AiboCore::AiboCore(const char *ip_addr) : ThreadedDriver(cf, section)
{
    walk.connect(ip_addr);
    head.connect(ip_addr);
    //cam.connect(ip_addr);
    ++AiboCore::aibo_count;
}*/

void AiboCore::connect(char *ip_addr)
{

    walk.connect(ip_addr);
    head.connect(ip_addr);
    //cam.connect(ip_addr);

}

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

	//Create aibo device with it's ip as the argument
	//aibodev = this;


	memset(&position_addr, 0, sizeof(player_devaddr_t));
  	memset(&ptz_addr, 0, sizeof(player_devaddr_t));
	memset(&camera_addr, 0, sizeof(player_devaddr_t));

	//Position2d
	if(cf->ReadDeviceAddr(&position_addr, section, "provides", PLAYER_POSITION2D_CODE, -1, NULL) == 0)
	{
    		if(AddInterface(position_addr) != 0)
		{
			PLAYER_ERROR("Error adding Position2d interface\n");
			SetError(-1);
			return;
		}
		puts("Added Position2d interface to Aibo");
	}
	else
	{
		PLAYER_ERROR("Error.  Constructor failed to read Position2d data from config file\n");
		SetError(-1);
		return;
	}
	
	// PTZ	
	if(cf->ReadDeviceAddr(&ptz_addr,section, "provides", PLAYER_PTZ_CODE, -1, NULL) == 0)
	{
    	if(AddInterface(ptz_addr) != 0)
		{
			PLAYER_ERROR("Error adding PTZ interface\n");
			SetError(-1);
			return;
		}
		puts("Added PTZ interface to Aibo");
	}
	else{
		PLAYER_ERROR("Error.  Constructor failed to read PTZ data from config file\n");
		SetError(-1);
		return;
	}
	
	// Camera
  	if(cf->ReadDeviceAddr(&camera_addr, section, "provides", PLAYER_CAMERA_CODE, -1, NULL) == 0)
   	{
	        if (AddInterface(camera_addr) != 0)
            {
               PLAYER_ERROR("Could not add Camera interface for Aibo");
               SetError(-1);
               return;
            }
	   		puts("Added Camera Interface to Aibo");
	
  	}	
	
  	ip = cf->ReadString(section, "ip", "192.168.2.155"); 					// 155 is default if non is provided
	printf("Using IP: %s \n", ip);

	// Create head, walk, cam objects
	//walk.connect(ip);
    //head.connect(ip);
    //cam.connect(ip);
    ++AiboCore::aibo_count;

	//  Now held in AiboCore, AiboWalk, and AiboHead
	main_com_port = cf->ReadInt(section, "mainPort", 10020);	    		// Main port
	//walk_com_port = cf->ReadInt(section, "walkRemotePort", 10050);  		// Walk port
	//head_com_port = cf->ReadInt(section, "headRemotePort", 10052);  		// Head port
	estop_com_port = cf->ReadInt(section, "estopPort", 10053);      		// Estop Port
	rawCam_com_port = cf->ReadInt(section, "rawCamPort", 10011);   			// Seg Cam Port 10012, raw 10011
	
}


// Set up the device.  Return 0 if things go well, and -1 otherwise.
int AiboCore::MainSetup()
{ 
	puts("Aibo driver initializing.");
	sleep(1);

	if((main_fd = aibo_sock(ip, main_com_port)) < 0){
		PLAYER_ERROR("Error creating main socket");
		SetError(-1);
	}
	puts("Main socket created\n");
	sleep(1);

	if((estop_fd = aibo_sock(ip, estop_com_port)) < 0){
		PLAYER_ERROR("Error creating main socket");
		SetError(-1);
	}	
	puts("Estop socket created\n");
	sleep(1);
	
    if(send_aibo_msg(main_fd, "!root \"TekkotsuMon\" \"Head Remote Control\"\r\n") < 0){
		PLAYER_ERROR("Error opening Head socket on Aibo");
		SetError(-1);
	}
	sleep(1);
	printf("head open");

	if(send_aibo_msg(main_fd, "!root \"TekkotsuMon\" \"Walk Remote Control\"\r\n") < 0){
		PLAYER_ERROR("Error opening Walk socket on Aibo");
		SetError(-1);
	}
	sleep(1);
	printf("walk open");
	
	if(send_aibo_msg(main_fd, "!root \"TekkotsuMon\" \"Raw Cam Server\"\r\n") < 0){
		PLAYER_ERROR("Error starting Raw Cam Server on Aibo");
		SetError(-1);
	}
	sleep(1);
	
	send_aibo_msg(main_fd, "!set vision.transport=tcp\r\n");
	
	if(send_aibo_msg(estop_fd, "start\n") < 0){
		PLAYER_ERROR("Error turning off EStop on Aibo");
		SetError(-1);
	}
	sleep(1);

	close(main_fd);

	cam = new AiboCam(ip, rawCam_com_port, 1);
	sleep(1); // JP added this on 01/13/2010

	// Message for checking status:
	puts("Aibo driver ready");

 	// Starts the main device thread.  Creates a new thread and executes
	// Aibo::Main() which contains the main loop for the driver.
	sleep(1);
	StartThread();

  	return 0;
}



// Shutdown the device
void AiboCore::MainQuit()
{
	puts("Shutting Aibo driver down");
	StopThread();
	// We need to set this up so that we don't have to restart the damn aibo everytime.
	//close(aibodev->main_fd);
  	//close(aibodev->walk_fd);
	//close(aibodev->head_fd);
	//close(aibodev->estop_fd);
	//close(aibodev->segCam_fd);	
	//aibodev->cam.~AiboCam();
	
	puts("Aibo driver has been shutdown");
	return;
}


// Process Messages
int AiboCore::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
	// Catch position2d
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, position_addr))
	{
		//Call the forward function should take an argument
		//Need to figure out how to seperate functions for forward,
		//strafe, and rotate.  Look at the data variable.
		//printf("Processing message for position2d\n");	
		assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
		position_cmd = *(player_position2d_cmd_vel_t *) data;

		// printf("Sending walking commands %f %f\n", position_cmd.vel.px, position_cmd.vel.pa);
		
		// Really these two should be in aibo_walk, but there is some sort of problem with the fd.
		// Using the Sockets object could fix this in one fell swoop
	
		walk.forward(position_cmd.vel.px);
		return 0;
	} 
	// Catch PTZ
	else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, ptz_addr))
    {

		return 0;
	}
	
	// Just return 0 for now	
	return 0;
}

// Main function for device thread
void AiboCore::Main() 
{
	puts("In main");
	// Preparation for Camera interface 
	player_camera_data_t camdata;
	memset(&camdata, 0, sizeof(camdata));

	camdata.width = 104;
	camdata.height = 80;    
	camdata.fdiv = 1;
	camdata.bpp = 24;
	camdata.format = PLAYER_CAMERA_FORMAT_RGB888;
	camdata.compression = PLAYER_CAMERA_COMPRESS_RAW;
	camdata.image_count = 104*80*3;

	int picSize;

	while(true){
    	// Test if we are supposed to cancel.  Need for proper shutdown of the thread
    	pthread_testcancel();

  		// Interact with the device, and push out the resulting data, using
	  	// Driver::Publish();

		//  This is used to publish position2d data
		player_position2d_data_t posData;
		memset(&posData, 0, sizeof(posData));		
	
		//posData.vel.px = vx;
		//posData.vel.py = aibodev->vy;
		//posData.vel.pa = aibodev->va;
	
		Publish(position_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE, 
				(void*) &posData, sizeof(posData), NULL);
		
	
		// Update Camera data returns size of image
    	picSize = cam->updateMMap(1);
		//printf("pic size is %d\n", picSize);
	
		// Allocate space for image or resize allocated space	
		if( camdata.image == NULL){
			camdata.image = (uint8_t *) malloc(camdata.image_count);
		}

		else{
			camdata.image = (uint8_t *) realloc(camdata.image, camdata.image_count);
		}
		
		// Retrieve image out of encapsulation	
		picture = cam->getImage();

		// Copy image to camera interface buffer
		memcpy(camdata.image, picture, camdata.image_count);
	
		// Publish to interface		
		Publish(camera_addr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
				(void*) &camdata, sizeof(camdata), NULL);   
	
			
		ProcessMessages();	
		usleep(25000);
	}
  
  	free(camdata.image);
}

/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    Aibo_Register(table);  
    return(0);
  }
}








