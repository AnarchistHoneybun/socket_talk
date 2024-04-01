//
// Created by vrin on 3/31/24.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <pthread.h>

int createTCPIpv4Socket() { return socket(AF_INET, SOCK_STREAM, 0); }

struct sockaddr_in* createIPv4Address(char *ip, int port) {

    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) ==0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET,ip,&address->sin_addr.s_addr);

    return address;
}


void startListeningAndPrintMessagesOnNewThread(int socketFD);

void messageTransferRoutine(int socketFD);

int main() {

    int socketFD = createTCPIpv4Socket();

    char serverIP[16];
    printf("Enter the server IP address: ");
    scanf("%15s", serverIP);

    struct sockaddr_in *address = createIPv4Address(serverIP, 2000);

    int result = connect(socketFD, address, sizeof(struct sockaddr_in));

    if(result == 0)
        printf("Connected to server\n");
    else {
        printf("Failed to connect to server\n");
        return 1;
    }

//    // ask for username
//    char* username = NULL;
//    size_t usernameLen = 0;
//    printf("Enter your username:\n");
//    ssize_t usernameCount = getline(&username, &usernameLen, stdin);
//    // add null terminator
//    username[usernameCount-1] = '\0';

    // ask for username
    char username[10];
    printf("Enter your username: ");
    scanf("%s", username);

    // send username to server
    send(socketFD, username, strlen(username), 0);



    char *message = NULL;
    size_t len = 0;



    printf("Begin chatting. Type 'exit' to quit\n");

    startListeningAndPrintMessagesOnNewThread(socketFD);


    // char jointMessage[1024];
    while(1){

        ssize_t charCount = getline(&message, &len, stdin);
        // add null terminator
        message[charCount-1] = '\0';
        // add username to message
        // sprintf(jointMessage, "[%s] %s", username, message);

        if (charCount > 1) {
            if (strcmp(message, "exit") == 0) {
                send(socketFD, message, strlen(message), 0);
                break;
            } else {
                send(socketFD, message, strlen(message), 0);
            }
        }
        // free(message);
    }



    // free(username);
    close(socketFD);

    return 0;
}



void startListeningAndPrintMessagesOnNewThread(int socketFD) {

    pthread_t id;
    pthread_create(&id, NULL, messageTransferRoutine, socketFD);

}

void messageTransferRoutine(int socketFD) {
    char buffer[1024];
    while(1){
        int bytesReceived = recv(socketFD, buffer, 1024, 0);
        buffer[bytesReceived] = '\0';

        if (bytesReceived > 0){
            if (strcmp(buffer, "exit") == 0) {
                break;
            } else {
                printf("%s\n", buffer);
            }
        }
    }

    close(socketFD);
}
