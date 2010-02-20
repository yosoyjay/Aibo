#include "aibo_net.h"

AiboNet::AiboNet(char aibo_ip[], unsigned int aibo_port)
{

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating socket");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(aibo_port);

    if (inet_pton(AF_INET, aibo_ip, &servaddr.sin_addr) <= 0)
    {
        perror("Error with inet_pton");
    }

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Error connecting to Aibo");
    }

    // JP: Added this on 01/15/2010 to disable Nagle's Algorithm . I had to include tcp.h , too.
    // We need to confirm that these lines are working.
    //int flag = 1;

    //if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) < 0)
    //{
     //   perror("Error enabling TCP_NODELAY");
    //}

    sleep(1);

}

int AiboNet::send_data(char command, float magnitude)
{
    // Pack data into a 5-byte array
    float *p = reinterpret_cast<float *>(&buffer[1]);
    buffer[0] = command;
    *p = magnitude;

    if(send(sockfd, (const void *)buffer, strlen(buffer), 0) < 0)
    {
        perror("Error sending message to Aibo");
        return -1;
    }
    else
    {
        return 0;
    }
}

AiboNet::~AiboNet(){
  close(sockfd);
}
