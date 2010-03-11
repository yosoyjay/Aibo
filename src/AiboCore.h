#ifndef AIBO_CORE_H
#define AIBO_CORE_H

#include <unistd.h>
#include <libplayercore/playercore.h>

#include "AiboWalk.h"
#include "AiboHead.h"
#include "aibo/Aibo.h"
//Commented out until code is cleaned up and AiboCam is actually implemented
//#include "AiboCam.h"

class AiboCore : public ThreadedDriver
{
public:
    AiboCore();
    AiboCore(const char *ip_addr);
		AiboCore(ConfigFile* cf, int section);
    void connect(char *ip_addr);
    int count();
    ~AiboCore();

    AiboWalk walk;
    AiboHead head;
    //AiboCam cam;

		/* Functions required for Player
		 * 
		 */
		virtual int  MainSetup();
		virtual void MainQuit();

		int ProcessMessage( QueuePointer &resp_queue, player_msghdr* hdr, void* data );

		


private:
    static int aibo_count;
    //id_array_struct

		/* Functions required for Player
		 *
		 */
		virtual void Main();
	
		// Device addresses for proxies used in constructor
		player_devaddr_t position_addr;
		player_devaddr_t ptz_addr;
		player_devaddr_t camera_addr;

		// IP/Ports - Probably should be moved?
		const char* ip;
		int main_com_port;
		int walk_com_port;
		int head_com_port;
		int estop_com_port;
		int segCam_com_port;
		int rawCam_com_port;

		// Just a pointer to images from AiboCam
		uint8_t* picture;

		// File Descriptors for socket.
		int main_fd;
		int walk_fd;
		int head_fd;
		int estop_fd;
		int segCam_fd;
		int rawCam_fd;

		// The Aibo object, yes this is stupid.
		// Need to rethink this.
		AiboCore* aibodev;

		// temporary place to hold AiboCam aibo
		AiboCam* cam;
		
		// Position2d proxy variables
		player_position2d_cmd_vel_t position_cmd;
};

// j - where the heck should these go?
Driver* Aibo_Init( ConfigFile* cf, int section);

void Aibo_Register( DriverTable *table);

// The two functions below are for main & estop

int send_aibo_msg(int sockfd, const char *buffer){
  if( send(sockfd, (const void *)buffer, strlen(buffer), 0) < 0) {
		perror("Error sending message to Aibo");
		return -1;
  }
  else{
		return 0;
  }
}// End send_walk_cmd()


/*	These are required until a new stick is made where we don't
 *	have to open up ports.  It is still useful to have a mainPort
 *	to have the ability to send commands to Tekkotsu.
 */

// Create a tcp socket... return socket segCam_fd.
int aibo_sock(const char *server_ip, unsigned int server_port)
{
	int sockfd;
	struct sockaddr_in servaddr;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Error creating socket");
		return -1;
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(server_port);

	if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0){
		perror("Error with inet_pton");
		return -1;
	}
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0){
		perror("Error connecting to Aibo.. aibo_driver.h");
		return -1;
	}
	// JP: Added this on 01/15/2010 to disable Nagle's Algorithm . I had to include tcp.h , too.
	// We need to confirm that these lines are working.
	int flag = 1;
	if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)) < 0) {
		perror("Error enabling TCP_NODELAY");
	}
	else{
		//printf("TCP_NODELAY disabled");
	}
	sleep(1);

	return sockfd;
}

#endif
