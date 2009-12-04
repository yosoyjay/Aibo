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

