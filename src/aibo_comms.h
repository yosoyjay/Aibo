/*
 * aibo_comms.h
 *
 * Functions for communicating with the Aibo from the Player driver
 *
 * Copyright (C) 2010 - Aibo Team - Jesse Lopez - Pablo Munoz - Joel Gonzalez
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
#include "unp.h"

// JP: Added this on 01/15/2010
#include <netinet/tcp.h>

typedef struct
{
	// Aibo's IP... fix IP_SIZE I just made it up... 
	char *ip;	//IP address
	
	//socket information
	struct sockaddr_in servaddr;
	//socketlen_t len;
	
	// File Descriptors for socket.
	int main_fd;
	int walk_fd;
	int head_fd;
	int estop_fd;

	// Position2d variables
	double vx;					
	double vy;
	double va;

	// JP: Added this on 01/12/2010
	// Ptz variables
	double pan;
	double tilt;
    double roll;

} aibo_comm_t;

typedef struct
{
	char command;
	float amount;
} comm_t;

// Create Aibo
aibo_comm_t* aibo_create(const char *ip);

// Create socket to Aibo
int aibo_sock(const char *aibo_ip, unsigned int aibo_port);

// Aibo Walk
int aibo_walk( aibo_comm_t* aibo, float position_cmd_vel_px, float position_cmd_vel_pa);

// Aibo Head . Added by JP on 01/13/2010
// int aibo_head( aibo_comm_t* aibo, float  IMCOMPLETE

// Send commands to the Aibo
int send_aibo_msg(int sockfd, const char *buffer);

#endif
