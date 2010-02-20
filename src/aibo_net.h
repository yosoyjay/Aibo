#ifndef AIBO_NET
#define AIBO_NET

#include <netinet/tcp.h>
#include "unp.h"

class AiboNet
{
public:
    AiboNet(char ip_addr[], unsigned int aibo_port);
    int send_data(char command, float magnitude);
    int send_data(char command[], float magnitude[], int size);
    ~AiboNet();

private:
    int sockfd;
    struct sockaddr_in servaddr;
    char *buffer;
};

#endif
