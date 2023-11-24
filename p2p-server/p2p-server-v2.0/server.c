#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define REGISTER 'R'
#define ONLINE 'O'
#define ERROR 'E'
#define ACKNOWLEDGEMENT 'A'
#define SEARCH 'S'
#define DEREGISTER 'T'
#define QUIT 'Q'

struct pdu {
	char type;
	char data[100];
};
struct registerPDU {
	char type;
	char peerName[10];
	char contentName[10];
	struct	sockaddr_in addr;
};
struct content {
	char user[10];
	char file[10];
	struct	sockaddr_in addr;
	int port;
};

int main(int argc, char *argv[]){
	struct sockaddr_in sin;			// Internet End Address
	struct sockaddr_in fsin;		// Address of a client
	int s; 							// Index Server Socket
	int alen;						// From-Address Length (Client)
	int port=3000;					
	int i, n;
	
	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}	
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "[-]Can't create socket \n");
	}
	if(bind(s,(struct sockaddr*)&sin, sizeof(sin)) < 0){
		fprintf(stderr, "[-]Can't bind socket to port {%d}", port);
	}	
	
	printf("----------------------------------------------------------- \n");
	printf("[+] Setting up P2P server on port %d \n", port);
	printf("[+] P2P Server Setup Complete: Address --> %s \n", inet_ntoa(sin.sin_addr));
	printf("----------------------------------------------------------- \n");
	
	listen(s,5);
	alen = sizeof(fsin); 
	
	
	struct registerPDU rpdu, dpdu;								//Register & Download PDU's
	struct pdu spdu;											//Send PDU
	struct content registeredContent[10]; 						//Registered Content List
	int content_tracking[] = {0,0,0,0,0,0,0,0,0,0};	
	int found = 0, placement = 0, registered = 0;
	int errorFound = 0; 			
	
	
	
	while(1){
		if ((n = recvfrom(s, &rpdu, sizeof(rpdu), 0, (struct sockaddr *)&fsin, &alen)) < 0)	// Get PDU
			fprintf(stderr, "recvfrom error\n");
		
		switch(rpdu.type){
			case REGISTER:
				printf("[+] Client sent REGISTER request \n");
				printf("[+] Received: \n");
				printf("	[+] Peer Name: %s \n", rpdu.peerName);
				printf("	[+] Content Name: %s \n", rpdu.contentName);
				for(i = 0; i < 10; i++){
					if((strcmp(registeredContent[i].user, rpdu.peerName) == 0) && (strcmp(registeredContent[i].file, rpdu.contentName) == 0) && (content_tracking[i] == 1)){
						printf("[-]Error: Content already registered \n");
						errorFound = 1; 
					} 		
				}
				if(errorFound == 0){
					registered = 0; 
					for(i = 0; i < 10; i++){
						printf("i: %d, registered: %d, content_tracking[%d]: %d\n", i, registered, i, content_tracking[i]);
						if ((content_tracking[i]==0) && (registered ==0) && strcmp(registeredContent[i].file, rpdu.contentName) != 0){
							strcpy(registeredContent[i].user, rpdu.peerName);
							strcpy(registeredContent[i].file, rpdu.contentName);
							registeredContent[i].addr = rpdu.addr;
							registeredContent[i].port = rpdu.addr.sin_port;
							registered = 1;
							content_tracking[i] = 1;
							break;
						}					
					}
				}
				printf("Register Content: ");
				for(i = 0; i < 10; i++){
					printf("%d ", content_tracking[i]);
				}	
				printf("\n");
				if(registered == 1){
					spdu.type = ACKNOWLEDGEMENT;
					strcpy(spdu.data, "[+] Content has been registered \n");
					if(sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
						fprintf(stderr, "Error Sending REGISTER data \n");
						exit(1);
					}
				}
				if(registered == 0 && errorFound == 1){
					errorFound = 0; 
					spdu.type = ERROR;
					strcpy(spdu.data, "[-]Error: Content already registered \n");
					if(sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
						fprintf(stderr, "Error Sending ERROR REGISTER data \n");
						exit(1);
					}
				}
				break;			
			
			case ONLINE:
				found = 0; 
				printf("[+] Client sent ONLINE request \n");
				memset(spdu.data, '\0', sizeof(spdu.data));
				spdu.type = ONLINE;
				for(i = 0; i < 10; i++){
					if(content_tracking[i] == 1){
						strcat(spdu.data, registeredContent[i].file);
						strcat(spdu.data, "\n");
						found++; 
					}	
				}
				if(found == 0){
					spdu.type = ERROR;
					strcpy(spdu.data, "[-] Error no files registered");
				}	
				if(sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
					fprintf(stderr, "Error Sending ONLINE data \n");
					exit(1);
				}
				break;
				
			case SEARCH:
				found = 0; 
				printf("[+] Client sent SEARCH request \n");
				printf("[+] Received: \n");
				printf("	[+] Peer Name: %s \n", rpdu.peerName);
				printf("	[+] Content Name: %s \n", rpdu.contentName);
				for(i = 0; i < 10; i++){
					if(content_tracking[i] == 1){
						if(strcmp(registeredContent[i].user, rpdu.peerName) == 0){
							strcpy(dpdu.peerName, registeredContent[i].user);
							dpdu.addr = registeredContent[i].addr;
							found++;
						}	
					}	
				}	
				if(found == 0){
					dpdu.type = ERROR;
				} else {
					dpdu.type = SEARCH;
				}	
				if(sendto(s, &dpdu, sizeof(dpdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
					fprintf(stderr, "Error Sending ONLINE data \n");
					exit(1);
				}
				break;	
				
			case DEREGISTER:
				found = 0;
				printf("[+] Client sent DEREGISTER request \n");
				printf("[+] Received: \n");
				printf("	[+] Peer Name: %s \n", rpdu.peerName);
				printf("	[+] Content to Deregister: %s \n", rpdu.contentName);
				for(i = 0; i < 10; i++){
					if(content_tracking[i] == 1){
						if(strcmp(registeredContent[i].file, rpdu.contentName) == 0){
							content_tracking[i] = 0;
							memset(registeredContent[i].user, '\0', sizeof(registeredContent[i].user));
							memset(registeredContent[i].file, '\0', sizeof(registeredContent[i].file));
							registeredContent[i].port = 0;
							found++;
						}	
					}	
				}	
				if(found == 0){
					spdu.type = ERROR;
				} else {
					spdu.type = ACKNOWLEDGEMENT;
				}	
				if(sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
					fprintf(stderr, "Error Sending ONLINE data \n");
					exit(1);
				}
				break;
			
			case QUIT:
				found = 0; 
				printf("[+] Client sent QUIT request \n");
				for(i = 0; i < 10; i++){
					if(content_tracking[i] == 1){
						if(strcmp(registeredContent[i].user, rpdu.peerName) == 0){
							content_tracking[i] = 0;
							memset(registeredContent[i].user, '\0', sizeof(registeredContent[i].user));
							memset(registeredContent[i].file, '\0', sizeof(registeredContent[i].file));
							registeredContent[i].port = 0;
							found++;
						}	
					}	
				}	
				if(found == 0){
					spdu.type = ERROR;
				} else {
					spdu.type = ACKNOWLEDGEMENT;
				}	
				if(sendto(s, &spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, sizeof(fsin)) < 0){
					fprintf(stderr, "Error Sending ONLINE data \n");
					exit(1);
				}
				break;
		}	
		printf("----------------------------------------------------------- \n");	
	}
	
}
