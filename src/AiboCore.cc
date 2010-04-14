#include "AiboCore.h"


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
    }
    else
    {
        //PLAYER_ERROR("Error.  Constructor failed to read Position2d data from config file\n");
        SetError(-1);
        return;
    }

    // PTZ
    if(cf->ReadDeviceAddr(&ptz_addr,section, "provides", PLAYER_PTZ_CODE, -1, NULL) == 0)
    {
        if(AddInterface(ptz_addr) != 0)
        {
            //PLAYER_ERROR("Error adding PTZ interface\n");
            SetError(-1);
            return;
        }

        puts("Added PTZ interface to Aibo");
    }
    else
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

    ip = cf->ReadString(section, "ip", "192.168.2.155"); 					// 155 is default if non is provided
    printf("Using IP: %s \n", ip);
    protocol = cf->ReadInt(section, "protocol", 1);

    // Create head, walk, cam objects
    walk.connect(ip);
    head.connect(ip);
    cam.connect(ip);
    ++AiboCore::aibo_count;

    rawCam_com_port = cf->ReadInt(section, "rawCamPort", 10011);   			// Seg Cam Port 10012, raw 10011

}


// Set up the device.  Return 0 if things go well, and -1 otherwise.
int AiboCore::MainSetup()
{
    puts("Aibo driver initializing.");

    //cam = new AiboCam.connect(ip);
    cam.updateMMap(0);
    cam.initialize(cam.getWidth(),cam.getHeight(),3,0,1,2);
    sleep(1); // JP added this on 01/13/2010

    // Message for checking status:
    puts("Aibo driver ready");

    // Initialize mutex for walking
    pthread_mutex_init(&walk_mutex, NULL);

    // Starts the main device thread.  Creates a new thread and executes
    // Aibo::Main() which contains the main loop for the driver.
    StartThread();

    return 0;
}



// Shutdown the device
void AiboCore::MainQuit()
{
    puts("Shutting Aibo driver down");
    StopThread();
    //  Need to put destructors here?

    pthread_mutex_destroy(&walk_mutex);
    puts("Aibo driver has been shutdown");
    return;
}


// Process Messages
int AiboCore::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
    // Catch position2d
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, position_addr))
    {

        assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
        position_cmd = *(player_position2d_cmd_vel_t *) data;

        walk.walk(position_cmd.vel.px, position_cmd.vel.py, position_cmd.vel.pa);

        return 0;
    }
    // Catch PTZ
    else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, ptz_addr))
    {
        assert(hdr->size == sizeof(player_ptz_cmd_t));
        head_cmd = *(player_ptz_cmd_t *) data;

        head.move(head_cmd.pan, head_cmd.tilt, head_cmd.zoom);

        return 0;
    }

    // Just return 0 for now
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

    while(true)
    {
        // Test if we are supposed to cancel.  Need for proper shutdown of the thread
        pthread_testcancel();

        //  This is used to publish position2d data need PTZ as well
        player_position2d_data_t posData;
        memset(&posData, 0, sizeof(posData));

        posData.vel.px = position_cmd.vel.px;
        posData.vel.py = position_cmd.vel.py;
        posData.vel.pa = position_cmd.vel.pa;

        Publish(position_addr, PLAYER_MSGTYPE_DATA, PLAYER_POSITION2D_DATA_STATE,
                (void*) &posData, sizeof(posData), NULL);


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

        // Publish to interface
        Publish(camera_addr, PLAYER_MSGTYPE_DATA, PLAYER_CAMERA_DATA_STATE,
                (void*) &camdata, sizeof(camdata), NULL);


        ProcessMessages();
        // Driver manual and experience suggest that it's advantageous to sleep
        // so that Player doesn't get bogged down with Messages in the while loop.
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








