#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>                                                                            
#include <netdb.h>
#include <dirent.h>

char peerName[10], contentName[10];

// Structure of Protocol Data Unit (PDU)
struct sendpdu {
    char type;
    char peerName[10];
    char contentName[10];
    struct sockaddr_in addr;
};

struct datapdu {
	char type;
	char data[100];
};

// Definitions
#define BUFSIZE sizeof(struct PDU)
#define REGISTER 'R'
#define ONLINE 'O'
#define ERROR 'E'
#define ACKNOWLEDGEMENT 'A'
#define SEARCH 'S'
#define DOWNLOAD 'D'
#define DEREGISTER 'T'
#define QUIT 'Q'

// Function to print available actions
void printOptions() {
	printf("------------------------------------------------- \n");
	printf("1) Content Listing\n");
	printf("2) Content Registration\n");
	printf("3) Content Download\n");
	printf("4) Content De-Registration\n");
	printf("5) Quit\n");
    printf("------------------------------------------------- \n");
}

void getListofContent(int s){
	struct sendpdu spdu;
	struct datapdu rpdu;
	spdu.type = ONLINE;
	write(s, &spdu, sizeof(spdu));
	read(s, &rpdu, sizeof(rpdu));
	if(rpdu.type == ERROR){
		printf("%s \n", rpdu.data);
	} else {
		printf("Online Content:\n %s \n\n", rpdu.data);
	}		
	printf("Press ENTER to continue... \n");
}

void registerContent(int s, char peerName[], char contentName[], struct sockaddr_in reg_addr){
	struct sendpdu spdu;
	struct datapdu rpdu;
	
	// Checks if the file exists in the primary directory //
    if (access(contentName, F_OK) != 0) {
        printf("[-]This file does not exist\n");
        printf("Press ENTER to continue... \n");
        return;
    }
    
    spdu.type = REGISTER; 
	strcpy(spdu.peerName, peerName);
	strcpy(spdu.contentName, contentName);
    spdu.addr = reg_addr;
    write(s, &spdu, sizeof(spdu)); 
    read(s, &rpdu, sizeof(rpdu));
    printf("%s", rpdu.data);
    printf("Press ENTER to continue... \n");
	
}

void downloadContent(int s, char peerName[], char contentName[], struct sockaddr_in reg_addr){
	struct sendpdu spdu, rpdu;
	struct datapdu dpdu;
	int tcp_sock;
	
	spdu.type = SEARCH;
	strcpy(spdu.peerName, peerName);
	strcpy(spdu.contentName, contentName);
	spdu.addr = reg_addr;
	write(s, &spdu, sizeof(spdu));
	read(s, &rpdu, sizeof(rpdu));
	if(rpdu.type == ERROR){
		printf("[-] ERROR Can't find content specified \n");
		return;
	} else if(rpdu.type == SEARCH){
		spdu.type = DOWNLOAD;
		strcpy(spdu.peerName, peerName);
		strcpy(spdu.contentName, contentName);
		printf("Address Received. Establishing Connection With Content Server...\n");
		printf("Content Server Port: '%d'\n", rpdu.addr.sin_port);
		
		/* Create a socket */
        if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        	fprintf(stderr, "Can't create a socket\n");
        	exit(EXIT_FAILURE);
        }
	                                                                         
    	/* Connect to the content server */
        if (connect(tcp_sock, (struct sockaddr *)&rpdu.addr, sizeof(rpdu.addr)) == -1){
        	fprintf(stderr, "Can't Connect: %s (%d)\n", strerror(errno), errno);
        	exit(EXIT_FAILURE);
        }
        
        printf("Content Server Connection Established!\n");
	}	
		
}

void deregisterContent(int s, char contentName[]){
	struct sendpdu spdu;
	struct datapdu rpdu;
	spdu.type = DEREGISTER;
	strcpy(spdu.contentName, contentName);
	write(s, &spdu, sizeof(spdu));
	read(s, &rpdu, sizeof(rpdu));
	if(rpdu.type == ERROR){
		printf("[-]Error Deregistering Content");
		return;
	} else {
		printf("[+] Content Successfully Deregistered \n");
	}	
	printf("Press ENTER to continue... \n");	
}

void quit(int s, char peerName[]){
	struct sendpdu spdu;
	struct datapdu rpdu;
	spdu.type = QUIT;
	strcpy(spdu.peerName, peerName);
	write(s, &spdu, sizeof(spdu));
	read(s, &rpdu, sizeof(rpdu));
	if(rpdu.type == ERROR){
		printf("[-]Error Quiting Content");
		return;
	} else {
		printf("[+] Successfully Quit \n");
	}	
	exit(0);		
}

void user_menu(int s, struct sockaddr_in reg_addr){
	char input[10];
	int input2; 
	if(peerName[0] == '\0'){
		scanf("%9s", peerName);
		printf("[+] Entered the username: %s \n", peerName);
	}	
	printOptions();
	printf("[+] Enter your option here: ");
	scanf("%d", &input2); 
	
	// Start of thes options
	switch (input2) {
		case 1:
			getListofContent(s);
			break;

		case 2:
			printf("------------------------------------------------- \n");
			printf("[+]Please enter the name of the content you'd like to register: ");
			scanf("%9s", input);
			strcpy(contentName, input);
			registerContent(s, peerName, contentName, reg_addr);
			break;
			
		case 3:
			printf("------------------------------------------------- \n");
			printf("[+]Enter the name of the Client to download from: ");
			scanf("%9s", input);
			strcpy(peerName, input);
			printf("[+]Enter the name of the file to download: ");
			scanf("%9s", input);
			printf("..... \n");
			strcpy(contentName, input);
			downloadContent(s, peerName, contentName, reg_addr);
			break; 	
			
		case 4:
			printf("------------------------------------------------- \n");
			printf("[+]Enter the content name you want to de-register: ");
			scanf("%9s", input);
			strcpy(contentName, input);
			deregisterContent(s, contentName);
			break;
			
		case 5:
			printf("------------------------------------------------- \n");
			printf("[+]Are you sure you want to quit? \n");
			printf("[+]yes /or no: ");
			scanf("%9s", input);
			if(strcmp(input, "yes") == 0 || strcmp(input, "Yes") == 0){
				quit(s, peerName);
				break;
			} else {
				break;
			}	
			
	}	

}



int main(int argc, char **argv){
	char *host = "localhost";
	int port = 3000;
	struct hostent *phe;
	struct sockaddr_in indexServer, client, reg_addr;
	int client_len, reg_len;
	int s, sd, new_sd;
	
	switch (argc) {
		case 1:
			break;
		case 2:
			host = argv[1];
		case 3:
			host = argv[1];
			port = atoi(argv[2]);
			break;
		default:
			fprintf(stderr, "usage: UDPtime [host [port]]\n");
			exit(1);			
	}	
	
	memset(&indexServer, 0, sizeof(indexServer));
	indexServer.sin_family = AF_INET;                                          
	indexServer.sin_port = htons(port);
	
    /* Map host name to IP address, allowing for dotted decimal */
	if ( phe = gethostbyname(host) ){
			memcpy(&indexServer.sin_addr, phe->h_addr, phe->h_length);
	}
	else if ( (indexServer.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
	fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	fprintf(stderr, "Can't create socket \n");
                                                                         
    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&indexServer, sizeof(indexServer)) < 0)
	fprintf(stderr, "Can't connect to %s \n", host);	
	
	/* TCP Socket Setup */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(EXIT_FAILURE);
	}
	bzero((char*)&reg_addr, sizeof(struct sockaddr_in));
	reg_addr.sin_family = AF_INET;
	reg_addr.sin_port = htons(0);
	reg_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (bind(sd,(struct sockaddr*)&reg_addr, sizeof(reg_addr)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(EXIT_FAILURE);
	}
	
	reg_len = sizeof(struct sockaddr_in);
	getsockname(sd,(struct sockaddr*)&reg_addr,&reg_len);
	listen(sd, 10);
	
	fd_set rfds, afds;
	int ret_sel;
	
	peerName[0] = '\0';
	printf("------------------------------------------------- \n");
	printf("---> Welcome to the P2P Server <--- \n");
	printf("------------------------------------------------- \n");
	printf("Enter your username: \n");
	
	while(1) {
		
		FD_ZERO(&afds);
       	FD_SET(0,&afds);	// Listening on stdin
       	FD_SET(sd, &afds);	// Listening on server TCP socket
		memcpy(&rfds, &afds, sizeof(rfds));
		
	    	if (ret_sel = select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0){
	    		printf("Select() Error\n");
	    		exit(EXIT_FAILURE);
	    	}
	    	
		if(FD_ISSET(sd, &rfds)) {	// Check server TCP socket
			client_len = sizeof(client);
			new_sd = accept(sd,(struct sockaddr*)&client,&client_len);
			if (new_sd >= 0) {		// New Accepted TCP socket
				//handle_client(new_sd);	// Handle download request
				close(new_sd);
				printf("Enter Command:\n");
			}
		}
		if(FD_ISSET(fileno(stdin), &rfds)) {
			user_menu(s, reg_addr);
		}
	}
	close(sd);
	exit(0);
}





















