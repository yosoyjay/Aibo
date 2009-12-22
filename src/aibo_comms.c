/* 
 * aibo_comms.c
 *
 * Functions for communicating with a Aibo robot from the Player driver.
 *
 *  Copyright (C) 2009 - Jesse Lopez
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
 *
 */

#include "aibo_comms.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

aibo_comm_t* aibo_create(const char *ip)
{
	aibo_comm_t *ret = (aibo_comm_t *) malloc(sizeof(aibo_comm_t));

	strncpy(ret->ip, ip, sizeof(ret->ip) - 1);

	ret->vx = 0.0;
	ret->vy = 0.0;
	ret->va = 0.0;

	return ret;
}

// Walk command
int aibo_walk( aibo_comm_t* aibo, float x, float a)
{
	char command, commandTwo;
	float forward, rotate;
	forward = 0;
	rotate = 0;
	
	//If just walking forward or backward
	if(x != 0 && a == 0){
		command = 'f';
		if(x < -0.6)
			forward = -0.8;
		else if( x < -0.4)
			forward = -0.5;
		else if( x < 0)
			forward = -0.2;
		else if ( x < 0.3)
			forward = 0.2;
		else if ( x < 0.6)
			forward = 0.5;
		else
			forward = 0.8;	
	
		// Call function that actually sends the command to the Aibo
		send_walk_cmd( aibo, command, forward);
	}
	
	//If just rotating
	else if(x == 0 && a != 0){
		command = 'r';

		if(x < -0.6)
			rotate = -0.8;
		else if( x < -0.4)
			rotate = -0.5;
		else if( x < 0)
			rotate = -0.2;
		else if ( x < 0.3)
			rotate = 0.2;
		else if ( x < 0.6)
			rotate = 0.5;
		else
			rotate = 0.8;
		// Call function that actually sends the command to the Aibo
		send_walk_cmd( aibo, command, rotate);
	}
	// Implement command with rotation and foward
	else{
		command = 'f';
		if(x < -0.6)
			forward = -0.8;
		else if( x < -0.4)
			forward = -0.5;
		else if( x < 0)
			forward = -0.2;
		else if ( x < 0.3)
			forward = 0.2;
		else if ( x < 0.6)
			forward = 0.5;
		else
			forward = 0.8;	
	
		commandTwo = 'r';
		if(x < -0.6)
			rotate = -0.8;
		else if( x < -0.4)
			rotate = -0.5;
		else if( x < 0)
			rotate = -0.2;
		else if ( x < 0.3)
			rotate = 0.2;
		else if ( x < 0.6)
			rotate = 0.5;
		else
			rotate = 0.8;

		send_walk_cmd( aibo,command, forward);
		send_walk_cmd( aibo,commandTwo, rotate);
	}	

	//If not these should I write an error to Player?
	return -1;	
}

// Function that actually sends the commands to the Aibo
int send_walk_cmd(aibo_comm_t* aibo, char command, float amount){
	
	// Populate the struct to send data [This would be padded right?]
	// I'm not sure that this will work at all.	
	comm_t move;
	move.command = command;
  move.amount = amount;

	// Want to write 5 bytes of struct to socket... padding issue
	if(write(aibo->walk_fd, (void *)&move, 5) < 5){
		perror("Error writing to socket");
	}

	return 0;
}
	




