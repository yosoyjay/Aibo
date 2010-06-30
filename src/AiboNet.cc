#include "AiboNet.h"

const int DATA_SIZE = 5;
const int MAX_BUFF_SIZE = 10000;

AiboNet::AiboNet(const char *aibo_ip, unsigned int aibo_port,
	const int proto)
{
  if(proto == TCP_PROTO){
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

    int flag = 1;

    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void *) &flag, sizeof(int)) < 0)
    {
        perror("Error enabling TCP_NODELAY");
    }
  }else if(proto == UDP_PROTO){

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0); // JP: Changed this SOCK_STREAM, 0)) < 0); // It should be a datagram socket.
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

  }

    sleep(1);

}

int AiboNet::send_data(char command, float magnitude)
{
    // Pack data into a 5-byte array
    buffer = new char[DATA_SIZE];
    float *p = reinterpret_cast<float *>(&buffer[1]);
    buffer[0] = command;
    *p = magnitude;

    if(send(sockfd, (const void *)buffer, DATA_SIZE, 0) < 0)
    {
        perror("Error sending message to Aibo");
        return -1;
    }
    else
    {
        return 0;
    }
}

int AiboNet::send_data(char command[], float magnitude[], int size)
{
    buffer = new char[size*DATA_SIZE];
    int j = 0;

    // Pack data into a size byte array
    for(int i = 0; i < size; j += DATA_SIZE, ++i)
    {
        float *p = reinterpret_cast<float *>(&buffer[j+1]);
        buffer[j] = command[i];
        *p = magnitude[i];
    }

    if(send(sockfd, (const void *)buffer, (size*DATA_SIZE), 0) < 0)
    {
        perror("Error sending message to Aibo");
        return -1;
    }
    else
    {
        return 0;
    }
}

int AiboNet::send_data(const char *command)
{
	if(send(sockfd, (const void *) command, strlen(command), 0) < 0){
		return -1;
	} else
		return 0;
}

char *AiboNet::read(int count)
{
    static char buf[MAX_BUFF_SIZE];
    char ch[4];
    bzero(buf, MAX_BUFF_SIZE);
    int numbytes;

    for (int i = 0; i < count; ++i)
    {

        numbytes = recv(sockfd, ch, 1, 0);

        if (numbytes == 1)
        {
            buf[i] = ch[0];
        }
        else
        {
            i--;
        }
    }

    return buf;
}

char *AiboNet::readUntil(char stop)
{
    static char retval[MAX_BUFF_SIZE];
    int numbytes = 0;
    char ch[5];
    int pos = 0;
    numbytes = recv(sockfd, &ch, 1, 0);

    while (ch[0] != stop && numbytes == 1 && pos < 50)   // no text is > 50
    {
        retval[pos++] = ch[0];
        numbytes = recv(sockfd, &ch, 1, 0);
    }

    retval[pos] = 0; // end of string
    //printf("readUntil: read %d chars\n", pos);
    return retval;
}


AiboNet::~AiboNet()
{
    close(sockfd);
    delete [] buffer;
}
