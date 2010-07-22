#ifndef AIBO_NET_H
#define AIBO_NET_H

#include <netinet/tcp.h>
#include "unp.h"

const int TCP_PROTO = 1;
const int UDP_PROTO = 0;

/**
 * Responsible for low-level communcations.  Constructor creates
 * socket connection. 
 */
class AiboNet
{
public:

    AiboNet(const char *ip_addr, unsigned int aibo_port,
	    const int proto = TCP_PROTO);

	// Sends a single command to the Aibo.  Used for the head.
    int send_data(char command, float magnitude);

	// Sends an array of commands to the Aibo.  Used for walking.
    int send_data(char command[], float magnitude[], int size);

	// Sends a string to the Aibo.  Used for Estop. Can be used to send
	// other commands to Tekkotsu.
    int send_data(const char *command);

	// Read count bytes from Aibo
    char *read(int count);

	// Read count bytes from Abio, but you pass a pointer to a buffer.
	char *readState(int count, char* buf);

	// Read until you see a delimiter. Used to read strings from Aibo.
    char *readUntil(char stop);

    ~AiboNet();

private:
	
	// Socket file descriptor
    int sockfd;

	// Server information
    struct sockaddr_in servaddr;

	// Buffer used to read stuff from the Aibo
    char *buffer;
};

#endif
