// server.c code modified on November 13th by Sanjam
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

// Structure of the Protocol Data Unit (PDU)
struct PDU {
    char type;
    char data[100];
};

// Structure to store registered content
typedef struct {
    char content[10];
    char peer[10];
    char address[10];
    char portNumber[10];
} RegistrationData;

// Definitions
#define BUFSIZE sizeof(struct PDU)
#define ERROR 'E' // report error
#define ONLINE 'O'
#define REGISTER 'R'
#define ACKNOWLEDGEMENT 'A'

// Main section //
int main(int argc, char *argv[]) {
    // Variables
    struct sockaddr_in clientAddress;
    char buffer[100];
    char *pointerToString;
    int serverSocket;
    time_t currentTime;
    int addressLength;
    struct sockaddr_in serverAddress;
    int socketDescriptor, socketType, errorFlag = 0;
    int serverPort = 3000;

    struct PDU receivedMsg, sendMsg;
    int socketReadSize, i = 0;
    char socketBuffer[BUFSIZE];
    char onlineContent[100];
    int contentRegistered = 0, contentFound = 0, position = 0;
    RegistrationData registeredContent[10];
    char receivedData[100], peerName[10], contentName[10], address[10], portNum[10];
    int contentStatusTracking[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    // Set up UDP server connection
    switch (argc) {
        case 1:
            break;
        case 2:
            serverPort = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            exit(1);
    }

    // Set up server address structure
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(serverPort);

    // Create a UDP socket for communication with clients
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        fprintf(stderr, "Can't create socket\n");
    }

    // Bind the socket to the server address
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Can't bind to %d port\n", serverPort);
    }

    listen(serverSocket, 5);
    addressLength = sizeof(clientAddress);

    // Main section, after UDP connection is set up
    while (1) {
        memset(&sendMsg, '\0', sizeof(sendMsg));
        memset(&receivedData, '\0', sizeof(receivedData));
        contentRegistered = 0;
        contentFound = 0;
        errorFlag = 0;

        // Receive a message from a client
        if (recvfrom(serverSocket, socketBuffer, sizeof(socketBuffer), 0, (struct sockaddr *)&clientAddress, &addressLength) < 0) {
            printf("recvfrom error\n");
        }

        memcpy(&receivedMsg, socketBuffer, sizeof(socketBuffer));

        // Handle request for list of available content
        if (receivedMsg.type == ONLINE) {
            // Prepare message to send to client
            sendMsg.type = ONLINE;
            for (i = 0; i < 10; ++i) { // Get list of existing content, separate by commas
                if (contentStatusTracking[i] == 1) {
                    strcat(sendMsg.data, registeredContent[i].content);
                    strcat(sendMsg.data, ", ");
                }
            }
            // Send message to client
            sendto(serverSocket, &sendMsg, BUFSIZE, 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
        }
        // Handle request to register content
        else if (receivedMsg.type == REGISTER) {
            // Get and parse message
            printf("The client wants to register new data\n");
            memcpy(receivedData, receivedMsg.data, BUFSIZE - 1);
            sscanf(receivedData, "%[a-zA-Z0-9];%[a-zA-Z0-9];%[0-9];%[0-9]", peerName, contentName, address, portNum);
            // Print information received
            printf("Peer name: %s\n", peerName);
            printf("Content name: %s\n", contentName);
            printf("Address: %s\n", address);
            printf("Port: %s\n", portNum);
            // Find an empty spot to store the content information
            for (i = 0; i < 10; ++i) {
                if ((contentStatusTracking[i] == 0) && (contentRegistered == 0)) {
                    strcpy(registeredContent[i].content, contentName);
                    strcpy(registeredContent[i].peer, peerName);
                    strcpy(registeredContent[i].address, address);
                    strcpy(registeredContent[i].portNumber, portNum);
                    contentRegistered = 1;
                    contentStatusTracking[i] = 1;
                }
            }
            // Send an acknowledgement if the content was stored properly
            if (contentRegistered == 1) {
                sendMsg.type = ACKNOWLEDGEMENT;
                strcpy(sendMsg.data, "Content has been registered");
            }
            // Send an error if the content couldn't be stored
            else if (contentRegistered == 0) {
                sendMsg.type = ERROR;
                strcpy(sendMsg.data, "Error registering content");
            }
            // Send message to the client
            sendto(serverSocket, &sendMsg, BUFSIZE, 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress));
        }
    }
    // Close the socket and exit the program
    close(serverSocket);
    exit(0);
}

