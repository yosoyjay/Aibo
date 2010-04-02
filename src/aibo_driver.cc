/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003
 *     Brian Gerkey
 *
 *	Copyright (C) 2010  Aibo Team Jesse Lopez - Pablo Munoz - Joel Gonzalez
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


// ONLY if you need something that was #define'd as a result of configure
// (e.g., HAVE_CFMAKERAW), then #include <config.h>, like so:
/*
#include <config.h>
*/

#include <string.h>
#include "aibo_driver.h"

// A factory creation function.  In this function, we create and return
// (as a generic Driver*) a pointer to a new instance of this driver.
Driver*
Aibo_Init(ConfigFile* cf, int section)
{
    // JP: Comments:
    printf("JP: in Aibo_Init() Aibo Driver Registered\n");

    // Create and return a new instance of this driver
    return((Driver*)(new Aibo(cf, section)));
}

// A driver registration function.  In this function, we add
// the driver into the given driver table, indicating which interface the
// driver can support and how to create a driver instance.
void Aibo_Register(DriverTable* table)
{
    table->AddDriver("aibo", Aibo_Init);
}

////////////////////////////////////////////////////////////////////////////////
// Constructor.  Retrieve options from the configuration file and do any
// pre-Setup() setup.
Aibo::Aibo(ConfigFile* cf, int section)
    : ThreadedDriver(cf, section)//, true, PLAYER_MSGQUEUE_DEFAULT_MAXLEN)
{
    // Clear memory of the proxy pointers
    memset(&position_addr, 0, sizeof(player_devaddr_t));
    memset(&ptz_addr, 0, sizeof(player_devaddr_t));

    // JP: Comments
    printf("JP: in Aibo constructor\n");

    // Check if the configuration file asks to provide position2d interface
    if(cf->ReadDeviceAddr(&position_addr, section, "provides", PLAYER_POSITION2D_CODE, -1, NULL) == 0)
    {
        // puts("Adding Position2d");
        // If the interface failed to correctly register
        if((AddInterface(position_addr)) != 0)
        {
            // Post an error string and quit constructor
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

    // Check if the configuration file asks to provide ptz interface
    if(cf->ReadDeviceAddr(&ptz_addr,section, "provides", PLAYER_PTZ_CODE, -1, NULL) == 0)
    {
        //puts("Adding PTZ");
        // If the interface failed to correctly register
        if(AddInterface(ptz_addr) != 0)
        {
            // Post an error string and quit constructor
            PLAYER_ERROR("Error adding PTZ interface\n");
            SetError(-1);
            return;
        }

        puts("Added PTZ interface to Aibo");
    }
    else
    {
        PLAYER_ERROR("Error.  Constructor failed to read PTZ data from config file\n");
        SetError(-1);
        return;
    }

    // Read of type String to get IP and set up ports
    // JP: 192.168.2.3 is for testing on simulator
    ip = cf->ReadString(section, "ip", "192.168.2.155");

    //JP: Comments. JL: Nice/Useful
    printf("Using IP: %s \n", ip);

    main_com_port = cf->ReadInt(section, "mainPort", 10020);	    // Main port
    walk_com_port = cf->ReadInt(section, "walkRemotePort", 10050);  // Walk port
    head_com_port = cf->ReadInt(section, "headRemotePort", 10052);  // Head port
    estop_com_port = cf->ReadInt(section, "estopPort", 10053);      // Estop Port

    printf("Constructor finished.\n");
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
// jesse: this is where communication set-up belongs create sockets and open ports
int Aibo::MainSetup()
{
    //Create aibo device with it's ip as the argument
    aibodev = aibo_create(ip);
    puts("Aibo driver initializing.");

    // Here you do whatever is necessary to setup the device, like open and
    // configure a serial port.

    //Create socket for Main control
    //Write commands to write commands to the port to open the appropriate walk/head ports.
    //Thereafter, create/connect to sockets or walking[10050], head[10052], estop[10053]

    // Added sleep after creating each socket as in the walk_fd, head_fd below.
    // I'm hoping this may eliviate some problems with starting up the program
    if((aibodev->main_fd = aibo_sock(ip, main_com_port)) < 0)
    {
        PLAYER_ERROR("Error creating main socket");
        SetError(-1);
    }

    puts("main created");
    sleep(3);

    if((aibodev->estop_fd = aibo_sock(ip, estop_com_port)) < 0)
    {
        PLAYER_ERROR("Error creating main socket");
        SetError(-1);
    }

    puts("estop created");
    sleep(3);

    // Send commands to Tekkotsu to open up Head, Walk, Estop ports
    if(send_aibo_msg(aibodev->main_fd, "!root \"TekkotsuMon\" \"Head Remote Control\"\r\n") < 0)
    {
        PLAYER_ERROR("Error opening Head socket on Aibo");
        SetError(-1);
    }

    sleep(3);

    if(send_aibo_msg(aibodev->main_fd, "!root \"TekkotsuMon\" \"Walk Remote Control\"\r\n") < 0)
    {
        PLAYER_ERROR("Error opening Walk socket on Aibo");
        SetError(-1);
    }

    sleep(3);

    if(send_aibo_msg(aibodev->estop_fd, "start\n") < 0)
    {
        PLAYER_ERROR("Error turning off EStop on Aibo");
        SetError(-1);
    }

    sleep(3);

    // After the ports have been opened for walking/head we can create socket
    aibodev->walk_fd = aibo_sock(ip, walk_com_port);
    sleep(3); // JP added this on 01/13/2010

    aibodev->head_fd = aibo_sock(ip, head_com_port);
    sleep(3); // JP added this on 01/13/2010

    // Starts the main device thread.  Creates a new thread and executes
    // Aibo::Main() which contains the main loop for the driver.
    StartThread();

    // Message for checking status:
    puts("Aibo driver ready");

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the device
void Aibo::MainQuit()
{
    puts("Shutting Aibo driver down");
    StopThread();

    puts("Aibo driver has been shutdown");
    return;
}

/////////////////////////////////////////////////////////////////////////////////
// Process Messages
int Aibo::ProcessMessage(QueuePointer &resp_queue, player_msghdr *hdr, void *data)
{
    // Process messages here.  Send a response if necessary, using Publish().
    // If you handle the message successfully, return 0.  Otherwise,
    // return -1, and a NACK will be sent for you, if a response is required.
    if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, position_addr))
    {
        //Call the forward function should take an argument
        //Need to figure out how to seperate functions for forward,
        //strafe, and rotate.  Look at the data variable.
        printf("Processing message for position2d\n");
        assert(hdr->size == sizeof(player_position2d_cmd_vel_t));
        position_cmd = *(player_position2d_cmd_vel_t *) data;
        // I don't know what this function does.
        //PLAYER_MSG2(2, "Sending walking commands %f %f", position_cmd.vel.px, position_cmd.vel.pa);

        if(!aibo_walk(aibodev, position_cmd.vel.px, position_cmd.vel.pa))
        {
            PLAYER_ERROR("Failed to send walk commands.");
            return -1;
        }

        return 0;
    }
    // JP: Ptz message processing
    else if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_PTZ_CMD_STATE, ptz_addr))
    {
        // JP: Still working on it. I SHOULD PUT THIS IN A SEPARATE FUNCTION
        // JP: NEEDS ERROR CHECKING. SPECIALLY WRITING TO SOCKET AND CHECKING LIMITS OF JOINTS.
        // JP: RANGE JOINTS
        // Pan 1.0 to -1.0 Center is 0
        // Tilt 0 to -1 Center is 0
        // Roll THis should be called SECONDTILT 1 to -1 (I should double check the negative of this)
        // JP: Comment
        printf("Processing message for PTZ\n");
        assert(hdr->size == sizeof(player_ptz_cmd_t));
        player_ptz_cmd_t cmdPosCamera;
        cmdPosCamera = *(player_ptz_cmd_t *) data;

        aibodev->tilt = cmdPosCamera.tilt;
        aibodev->pan = cmdPosCamera.pan;
        aibodev->roll = cmdPosCamera.zoom; // JP: THIS IS NOT GOOD. I AM USING ZOOM AS ROLL. Find a better way to do it.
        char tiltch = 't';
        char panch = 'p';
        char rollch = 'r';

        // JP: DO NOT FORGET TO SET THE LIMITS ON THE MOVEMENTS!
        if(aibodev->tilt > 0.0)
            aibodev->tilt = 0.0;
        else if(aibodev->tilt < -1.0)
            aibodev->tilt = -1.0;

        if(aibodev->pan > 1.0)
            aibodev->pan = 1.0;
        else if(aibodev->pan < -1.0)
            aibodev->pan = -1.0;

        if(aibodev->roll > 1.0)
            aibodev->roll = 1.0;
        else if(aibodev->roll < -1.0)
            aibodev->roll = -1.0;

        printf("Pan %f Tilt %f Roll %f \n", aibodev->pan, aibodev->tilt, aibodev->roll);

        // Send tilt
        char outbuf[5];
        char * p = outbuf;
        memcpy(p, &tiltch, sizeof(tiltch));
        //p+=1;
        long convertedFloat = *reinterpret_cast<long *>(&aibodev->tilt);
        // The Tekkotsu way
        int i = ((convertedFloat >> 24) & 0xff);
        outbuf[4] = i;
        i = ((convertedFloat >> 16) & 0xff);
        outbuf[3] = i;
        i = ((convertedFloat >> 8) & 0xff);
        outbuf[2] = i;
        i = (convertedFloat & 0xff);
        outbuf[1] = i;

        if(send_aibo_msg(aibodev->head_fd, outbuf) < 0)
        {
            PLAYER_ERROR("Failed to send head commands.");
        }
        else
        {
            printf("Message Sent\n");
        }

        // Send pan
        p = outbuf;
        memcpy(p, &panch, sizeof(panch));
        //p+=1;
        convertedFloat = *reinterpret_cast<long *>(&aibodev->pan);
        // The Tekkotsu way
        i = ((convertedFloat >> 24) & 0xff);
        outbuf[4] = i;
        i = ((convertedFloat >> 16) & 0xff);
        outbuf[3] = i;
        i = ((convertedFloat >> 8) & 0xff);
        outbuf[2] = i;
        i = (convertedFloat & 0xff);
        outbuf[1] = i;

        if(send_aibo_msg(aibodev->head_fd, outbuf) < 0)
        {
            PLAYER_ERROR("Failed to send head commands.");
        }
        else
        {
            printf("Message Sent\n");
        }

        // Send roll
        p = outbuf;
        memcpy(p, &rollch, sizeof(rollch));
        //p+=1;
        convertedFloat = *reinterpret_cast<long *>(&aibodev->roll);
        // The Tekkotsu way
        i = ((convertedFloat >> 24) & 0xff);
        outbuf[4] = i;
        i = ((convertedFloat >> 16) & 0xff);
        outbuf[3] = i;
        i = ((convertedFloat >> 8) & 0xff);
        outbuf[2] = i;
        i = (convertedFloat & 0xff);
        outbuf[1] = i;

        if(send_aibo_msg(aibodev->head_fd, outbuf) < 0)
        {
            PLAYER_ERROR("Failed to send head commands.");
        }
        else
        {
            printf("Message Sent\n");
        }

        // TODO: Call function aibo_head to send commands to aibo
        return 0;
    }

    // JP: Changed this on 01/13/10 to return -1 from return 0.
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Aibo::Main()
{
    //JP: Comments
    puts("In Aibo::Main()\n");

    // The main loop; interact with the device here
    for(;;)
    {
        // test if we are supposed to cancel
        // JP: Commented this on 01/13/2010. Do we need it?
        //pthread_testcancel();

        // Process incoming messages.  Aibo ::ProcessMessage() is
        // called on each message.
        // JP: Comments
        // printf("Going to process messages\n");
        ProcessMessages();

        // Interact with the device, and push out the resulting data, using
        // Driver::Publish()
        /*
        player_position2d_data_t posData;
        memset(&posData, 0, sizeof(posData));

        posData.vel.px = this->aibodev->vx;
        posData.vel.py = this->aibodev->vy;
        posData.vel.pa = this->aibodev->va;

        this->Publish(this->position_addr, PLAYER_MSGTYPE_DATA,
        			PLAYER_POSITION2D_DATA_STATE, (void*) &posData, sizeof(posData),
        			NULL);
        */

        // Sleep (you might, for example, block on a read() instead)

        //sleep(1); // JP: I had it originally at 1. It worked on 0 but it was going back to 1 head position on pan
        usleep(60000);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C" {
    int player_driver_init(DriverTable* table)
    {
        Aibo_Register(table);  // This is a void function.
        puts("Aibo driver registered to driver table");
        return(0);
    }
}
