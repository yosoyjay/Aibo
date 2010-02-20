/*
 * aibo_comms.c
 *
 * Functions for communicating with a Aibo robot from the Player driver.
 *
 *  Copyright (C) 2010 - Aibo Team BC - Jesse Lopez - Pablo Munoz - Joel Gonzalez
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

#define _AIBO_ASM_ 1

aibo_comm_t* aibo_create(const char *ip)
{
    //JP: Comments
    //printf("In aibo_create\n");

    aibo_comm_t *ret = (aibo_comm_t *) malloc(sizeof(aibo_comm_t));

    strncpy(ret->ip, ip, sizeof(ret->ip) - 1);

    ret->vx = 0.0;
    ret->vy = 0.0;
    ret->va = 0.0;

    // JP: Added on 01/12/2010
    ret->pan = 0.0;
    ret->tilt = 0.0;

    return ret;
}

// Create a socket... return socket fd.
int aibo_sock(const char *server_ip, unsigned int server_port)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating socket");
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0)
    {
        perror("Error with inet_pton");
        return -1;
    }

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Error connecting to Aibo");
        return -1;
    }

    // JP: Added this on 01/15/2010 to disable Nagle's Algorithm . I had to include tcp.h , too.
    // We need to confirm that these lines are working.
    int flag = 1;

    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) < 0)
    {
        perror("Error enabling TCP_NODELAY");
    }
    else
    {
        printf("TCP_NODELAY disabled");
    }

    sleep(1);

    return sockfd;
}

// Walk command
int aibo_walk( aibo_comm_t* aibo, float x, float a)
{
    char command;
    char outbuf[5];
    float forward, rotate;
    forward = 0;
    rotate = 0;

    //If just walking forward or backward
    if(x != 0 && a == 0)
    {
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

        tekkotsu_command(outbuf, command, forward);
        send_aibo_msg(aibo->walk_fd, outbuf);
    }

    /*
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
    	//send_walk_cmd( aibo, command, rotate);
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

    	//send_walk_cmd( aibo,command, forward);
    	//send_walk_cmd( aibo,commandTwo, rotate);
    }

    //If not these should I write an error to Player?
    return -1;*/
    return 0;

}// End aibo_walk()

// Function that actually sends the commands to the Aibo
int send_aibo_msg(int sockfd, const char *buffer)
{
    if(send(sockfd, (const void *)buffer, strlen(buffer), 0) < 0)
    {
        perror("Error sending message to Aibo");
        return -1;
    }
    else
    {
        return 0;
    }
}// End send_walk_cmd()

// Convert Command and Float to Tekkotstu friendly format
int tekkotsu_command(char *buffer, char command, float amount)
{

#ifdef _AIBO_ASM_
    asm ( "bswap %%eax\t\n"
          : "+a"(amount):);

    memcpy(&buffer[1], &amount, sizeof(amount));
    buffer[0] = command;
#else
    buffer[0] = command;
    //p+=1;
    long convertedFloat = *reinterpret_cast<long *>(&amount);
    // The Tekkotsu way
    int i = ((convertedFloat >> 24) & 0xff);
    buffer[4] = i;
    i = ((convertedFloat >> 16) & 0xff);
    buffer[3] = i;
    i = ((convertedFloat >> 8) & 0xff);
    buffer[2] = i;
    i = (convertedFloat & 0xff);
    buffer[1] = i;

#endif

    return 0;

}
