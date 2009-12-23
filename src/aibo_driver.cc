/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2003  
 *     Brian Gerkey
 *                   
 *	Copyright (C) 2009   
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

/*
 * A simple example of how to write a driver that will be built as a
 * shared object.
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
    : ThreadedDriver(cf, section)
{
	memset(&this->position_addr, 0, sizeof(player_devaddr_t));
	memset(&this->ptz_addr, 0, sizeof(player_devaddr_t));


	// Check if the configuration file asks to provide position2d interface
	if(cf->ReadDeviceAddr(&(this->position_addr), section, "provides", PLAYER_POSITION2D_CODE, -1, NULL) == 0)
	{
		puts("Adding Position2d");
		// If the interface failed to correctly register 
    if((this->AddInterface(this->position_addr)) != 0)
		{
			// Post an error string and quit constructor
			PLAYER_ERROR("Error adding Position2d interface\n");
			this->SetError(-1);
			return;
		}
	}
	
	// Check if the configuration file asks to provide ptz interface
	if(cf->ReadDeviceAddr(&(this->ptz_addr),section, "provides", PLAYER_PTZ_CODE, -1, NULL) == 0)
	{
		puts("Adding PTZ");
		// If the interface failed to correctly register 
    if(AddInterface(ptz_addr) != 0)
		{
			// Post an error string and quit constructor
			PLAYER_ERROR("Error adding PTZ interface\n");
			SetError(-1);
			return;
		}
	}
		
	// Read of type Sting to get IP
  this->ip = cf->ReadString(section, "ip", "192.168.2.155");
  
	// Read port of Main
	this->main_com_port = cf->ReadInt(section, "mainPort", 10020);

	// Read port of Walk Remote Control
	this->walk_com_port = cf->ReadInt(section, "walkRemotePort", 10051);

	// Read port of EStop 
	this->estop_com_port = cf->ReadInt(section, "estopPort", 10053);
	
	printf("Constructor finished.\n");	

  return;
}


////////////////////////////////////////////////////////////////////////////////
// Set up the device.  Return 0 if things go well, and -1 otherwise.
// jesse: this is where communication set-up belongs create sockets and open ports
int Aibo::MainSetup()
{ 
	//Create aibo device with it's ip as the argument
	this->aibodev = aibo_create(this->ip);	
  puts("Aibo driver initializing...");

  // Here you do whatever is necessary to setup the device, like open and
  // configure a serial port.
	
	//Create socket for Main control 
	/*Write commands to write commands to the port to open the appropriate walk/head ports.  	Thereafter, create/connect to sockets or walking[10050], head[10052], estop[10053]*/

	aibodev->main_fd = aibo_sock(ip, main_com_port);
	aibodev->estop_fd = aibo_sock(ip, estop_com_port);

  if(send_aibo_msg(aibodev->main_fd, "!root \"TekkotsuMon\" \"Head Remote Control\"\r\n") < 0){
		PLAYER_ERROR("Error opening Head socket on Aibo");
		SetError(-1);
	}
	sleep(3);

	if(send_aibo_msg(aibodev->main_fd, "!root \"TekkotsuMon\" \"Walk Remote Control\"\r\n") < 0){
		PLAYER_ERROR("Error opening Walk socket on Aibo");
		SetError(-1);
	}
	sleep(3);

	if(send_aibo_msg(aibodev->estop_fd, "start\n") < 0){
		PLAYER_ERROR("Error turning off EStop on Aibo");
		SetError(-1);
	}
	sleep(3);

 	aibodev->walk_fd = aibo_sock(ip, walk_com_port);
	aibodev->head_fd = aibo_sock(ip, head_com_port);

	

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

  // Need to close sockets, free memory;

  puts("Aibo driver has been shutdown");
	return;
}

/////////////////////////////////////////////////////////////////////////////////
// Process Messages
int Aibo::ProcessMessage(QueuePointer & resp_queue, player_msghdr * hdr,
													void * data)
{
  // Process messages here.  Send a response if necessary, using Publish().
  // If you handle the message successfully, return 0.  Otherwise,
  // return -1, and a NACK will be sent for you, if a response is required.
	if(Message::MatchMessage(hdr, PLAYER_MSGTYPE_CMD, PLAYER_POSITION2D_CMD_VEL, this->position_addr))
	{

		//Call the forward function should take an argument
		//Need to figure out how to seperate functions for forward,
		//strafe, and rotate.  Look at the data variable.
		player_position2d_cmd_vel_t position_cmd;
		position_cmd = *(player_position2d_cmd_vel_t *) data;
		PLAYER_MSG2(2, "Sending walking commands %f %f", position_cmd.vel.px, position_cmd.vel.pa);

		if(!aibo_walk(this->aibodev, position_cmd.vel.px, position_cmd.vel.pa))
		{
			PLAYER_ERROR("Failed to send walk commands.");
		}
	}

		return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Main function for device thread
void Aibo::Main() 
{
  // The main loop; interact with the device here
  for(;;)
  {
    // test if we are supposed to cancel
    pthread_testcancel();

    // Process incoming messages.  Aibo ::ProcessMessage() is
    // called on each message.
    this->ProcessMessages();

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
    usleep(100000);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Extra stuff for building a shared object.

/* need the extern to avoid C++ name-mangling  */
extern "C" {
  int player_driver_init(DriverTable* table)
  {
    puts("Aibo driver initializing");
    Aibo_Register(table);
    puts("Aibo driver done");
    return(0);
  }
}
