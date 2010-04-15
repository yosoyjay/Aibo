#ifndef AIBO_NET_H
#define AIBO_NET_H

#include <netinet/tcp.h>
#include "unp.h"

//! \breif Class responsible for low level network communications
class AiboNet
{
public:
    AiboNet(const char *ip_addr, unsigned int aibo_port);
    // Created by JP to get a UPD socket
    AiboNet(const char *ip_addr, unsigned int aibo_port, int x);
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
