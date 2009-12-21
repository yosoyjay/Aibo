/*
 * aibo_comms.h
 *
 * Functions for communicating with the Aibo from the Player driver
 *
 * Copyright (C) 2009 - Jesse Lopez
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef	AIBO_COMMS_H_
#define	AIBO_COMMS_H_

//This is completely made up.  Fix this.
#define IP_SIZE 512

#include <libplayercore/playercore.h>


typedef struct
{
	// Aibo's IP... fix IP_SIZE I just made it up... 
	char ip[IP_SIZE];	//IP address
	
	// File Descriptors
	int main_fd;
	int walk_fd;
	int head_fd;

	// Position2d variables
	double vx;					
	double vy;
	double va;

} aibo_comm_t;

typedef struct
{
	char walk_t;
	float amount;
} comm_t;

// Create Aibo
aibo_comm_t* aibo_create(const char *port);

// Connect to Aibo

// Aibo Walk
int aibo_walk( aibo_comm_t* aibo, float position_cmd_vel_px, float position_cmd_vel_pa);

// Send commands to the Aibo
int send_walk_cmd(char walk_t, float amount);

 
	

#endif
