// client.c code modified on November 13th by Sanjam
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Structure of Protocol Data Unit (PDU)
struct PDU {
    char type;
    char data[100];
};

// Definitions
#define BUFSIZE sizeof(struct PDU)
#define REGISTER 'R'
#define ONLINE 'O'
#define ERROR 'E'
#define ACKNOWLEDGEMENT 'A'

// Universal variables
int clientSocket;
char peerName[10];

// Function to print available actions
void printOptions() {
    printf("\nPlease select one of the following actions to complete: \n");
    printf("'R' - register new content\n");
    printf("'O' - get list of content\n");
}

// Function to register new content
void registerContent(char content[], char peer[]) {
    if (access(content, F_OK) != 0) {
        printf("This file does not exist\n");
        return;
    }

    struct PDU msg, responseMsg;
    msg.type = REGISTER;
    // Construct the registration message
    snprintf(msg.data, sizeof(msg.data), "%s;%s;%s;%s", peer, content, "placeholder_address", "placeholder_port");
    // Send the registration message to the server
    write(clientSocket, &msg, BUFSIZE);

    // Receive and handle the server's response
    read(clientSocket, &responseMsg, BUFSIZE);

    if (responseMsg.type == ACKNOWLEDGEMENT) {
        printf("Content has been registered successfully\n");
    } else if (responseMsg.type == ERROR) {
        printf("Error: %s\n", responseMsg.data);
    }
}

// Function to get the list of registered content
void getListOfContent() {
    struct PDU msg, responseMsg;
    msg.type = ONLINE;
    strcpy(msg.data, "online content");
    // Send request for online content to the server
    write(clientSocket, &msg, BUFSIZE);

    // Receive and display the server's response
    read(clientSocket, &responseMsg, BUFSIZE);
    printf("Available Content: %s\n", responseMsg.data);
}

// Main part of the program
int main(int argc, char **argv) {
    char *serverHost = "localhost";
    int serverPort = 3000;
    struct sockaddr_in serverAddress;

    int socketReadSize;
    char socketReadBuf[BUFSIZE];
    struct PDU msg, responseMsg;
    char choice[1], contentName[10], address[80] = "placeholder_address", input[10], input2[10];

    // Parse command line arguments to get server host and port
    switch (argc) {
        case 1:
            break;
        case 2:
            serverHost = argv[1];
        case 3:
            serverHost = argv[1];
            serverPort = atoi(argv[2]);
            break;
        default:
            fprintf(stderr, "usage: UDP File Download [host [port]]\n");
            exit(1);
    }

    // Set up server address structure
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    // Get server information by host name or IP address
    struct hostent *serverInfo;
    if (serverInfo = gethostbyname(serverHost)) {
        memcpy(&serverAddress.sin_addr, serverInfo->h_addr, serverInfo->h_length);
    } else if ((serverAddress.sin_addr.s_addr = inet_addr(serverHost)) == INADDR_NONE) {
        fprintf(stderr, "Can't get host entry \n");
    }

    // Create a UDP socket for communication with the server
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        fprintf(stderr, "Can't create socket \n");
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Can't connect to %s\n", serverHost);
    }

    // Get the name of the peer on startup
    memset(&peerName, '\0', sizeof(peerName));
    printf("Enter Peer Name:\n");
    int readSize = read(0, peerName, BUFSIZE);
    if (readSize >= 10) {
        printf("ERROR\n");
        return -1;
    }
    peerName[strlen(peerName) - 1] = '\0';

    // Main loop of the program
    while (1) {
        memset(&input, '\0', sizeof(input));
        memset(&msg, '\0', sizeof(msg));

        // Display available actions to the user
        printOptions();
        // Read user's choice
        read(0, choice, 2);
        printf("Your choice is: %s\n", choice);

        // Start of the options
        if (choice[0] == 'O') { // ONLINE CONTENT
            getListOfContent();
        } else if (choice[0] == 'R') { // REGISTER CONTENT
            printf("Please enter the name of the content you'd like to register: ");
            // Read the content name from the user
            scanf("%9s", input);
            // Register the content with the server
            registerContent(input, peerName);
        }
    }
    // Close the socket and exit the program
    close(clientSocket);
    exit(0);
}

