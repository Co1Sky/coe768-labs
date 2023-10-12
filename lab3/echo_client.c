/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



#define SERVER_TCP_PORT 3000						/* well-known port */
#define PACKET_SIZE		100							/* packet size */

void receive_file(int sd, char *file_name){
	FILE *file = fopen(file_name, "wb"); 
	if (file == NULL){
		printf("File creation failed!"); 	
	}	
	
	char packet[PACKET_SIZE]; 
	int bytes_received;
	
	while(bytes_received = read(sd, packet, sizeof(packet) > 0)){
		if (packet[0] == 'E') {
			printf("Server Error: %s\n", packet + 1);
			remove(file_name); 
			break;
		}	
		fwrite(packet, 1, bytes_received, file); 
	}
	
	fclose(file);
}



int main(int argc, char **argv)
{
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host; 

	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}
	
	char file_name[256];
	printf("Enter the file name: ");
	scanf("%s", file_name); 
	write(sd, file_name, strlen(file_name)); 
	receive_file(sd, file_name);  	

	return(0);
}


