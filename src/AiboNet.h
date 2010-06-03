#ifndef AIBO_NET_H
#define AIBO_NET_H

#include <netinet/tcp.h>
#include "unp.h"

const int TCP_PROTO = 1;
const int UDP_PROTO = 0;

//! \brief Class responsible for low level network communications
class AiboNet
{
public:
    AiboNet(const char *ip_addr, unsigned int aibo_port,
	    const int proto = TCP_PROTO);
    int send_data(char command, float magnitude);
    int send_data(char command[], float magnitude[], int size);
    char *read(int count);
    char *readUntil(char stop);
    ~AiboNet();

private:
    int sockfd;
    struct sockaddr_in servaddr;
    char *buffer;
};

#endif
