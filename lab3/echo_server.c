/* A simple echo server using TCP */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>


#define SERVER_TCP_PORT 3000	/* well-known port */
#define PACKET_SIZE		100		/* packet size */

void reaper(int);
void send_file(int client_socket, char *file_name); 

int main(int argc, char **argv)
{
	int 	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;

	switch(argc){
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);

	while(1) {
	  client_len = sizeof(client);
	  new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
	  if(new_sd < 0){
	    fprintf(stderr, "Can't accept client \n");
	    exit(1);
	 }
	  
	 switch (fork()){
	  	  
	case 0:	
	// -- CHILD PROCESS -- //	
		(void) close(sd); 												
		char file_name[256]; 									// file name placeholder //
        read(new_sd, file_name, sizeof(file_name));				// server waits 
        printf("FILE RECEIVED CALLED: %s \n", file_name);
		FILE* file = fopen(file_name, "rb");					// checks to see if file exists //
		if (file == NULL) {
			printf("-- NO FILE FOUND -- \n"); 
			break;
		} 
		printf("SUCCESSFULY FOUND FILE: %s \n", file_name); 
		char packet[PACKET_SIZE];								// packet to transfer data //
		int bytes_read;											// counter to read how many

		// packet => memory block where the data is being stored
		// 1 => size of each element in bytes
		// PACKET_SIZE (100 bytes) => # of elements read
		// file => file to read from
		// returns => number of elements read from file
		while ((bytes_read = fread(packet, 1, PACKET_SIZE, file)) > 0) {
			printf("NUMBER OF BYTES READ: %d \n", bytes_read); 
			printf("Packet[0] = %s", packet); 
			write(new_sd, packet, bytes_read);
		}
		fclose(file);										
		(void) close(new_sd); 											
		exit(0);
	default:
	// -- PARENT PROCESS -- //		
		(void) close(new_sd);
		break;
	case -1:
		fprintf(stderr, "fork: error\n");
	  }
	}
}

/*	reaper		*/
void reaper(int sig) {
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}
