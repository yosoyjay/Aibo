#ifndef AIBO_NET_H
#define AIBO_NET_H

#include <netinet/tcp.h>
#include "unp.h"

//! \breif Class responsible for low level network communications
class AiboNet
{
public:
    AiboNet(char ip_addr[], unsigned int aibo_port);
    int send_data(char command, float magnitude);
    int send_data(char command[], float magnitude[], int size);
    char *read(int count);
    ~AiboNet();

private:
    int sockfd;
    struct sockaddr_in servaddr;
    char *buffer;
};

#endif
