//
// Created by vrin on 3/31/24.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>
#include <netdb.h> // for gethostbyname
#include <unistd.h> // for gethostname
#include <stdbool.h>
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

struct AcceptedSocket {
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool accepted;
};

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD);

void messageTransferRoutine(int socketFD);
void acceptNewConnection(int serverSocketFD);


void recieveAndPrintIncomingDataOnSeperateThread(struct AcceptedSocket *pSocket);

void sendReceivedMessageToAllClients(char *buffer, int originClientFD);

char* getServerIPAddress() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;

    // To retrieve hostname
    gethostname(hostbuffer, sizeof(hostbuffer));

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));

    return IPbuffer;
}

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    int clientAddressLength = sizeof(struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD, &clientAddress, &clientAddressLength);

    struct AcceptedSocket *acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->address = clientAddress;
    acceptedSocket->accepted = clientSocketFD>0;
    if(!acceptedSocket->accepted){
        acceptedSocket->error = clientSocketFD;
    }

    return acceptedSocket;
}

struct AcceptedSocket acceptedSockets[10];
int acceptedSocketCount = 0;

void startAcceptingIncomingConnections(int serverSocketFD) {

    while(1){
        if(acceptedSocketCount == 9){
            continue;
        }
        struct AcceptedSocket *clientSocket = acceptIncomingConnection(serverSocketFD);
        acceptedSockets[acceptedSocketCount++] = *clientSocket;
        recieveAndPrintIncomingDataOnSeperateThread(clientSocket);

    }



}


void recieveAndPrintIncomingDataOnSeperateThread(struct AcceptedSocket *clientSocket) {
    pthread_t id;
    pthread_create(&id, NULL, messageTransferRoutine, clientSocket->acceptedSocketFD);


}

void messageTransferRoutine(int socketFD) {
    char buffer[1024];
    while(1){
        memset(buffer, 0, 1024);
        int bytesReceived = recv(socketFD, buffer, 1024, 0);

        if (bytesReceived > 0){
            if (strcmp(buffer, "exit") == 0) {
                break;
            } else {
                printf("%s\n", buffer);
                sendReceivedMessageToAllClients(buffer, socketFD);
            }
        }
    }

    close(socketFD);
}

void sendReceivedMessageToAllClients(char *buffer, int originClientFD) {
    for(int i=0; i<acceptedSocketCount; i++){
        if(acceptedSockets[i].acceptedSocketFD != originClientFD){
            send(acceptedSockets[i].acceptedSocketFD, buffer, strlen(buffer), 0);
        }
    }

}

int main() {

    int serverSocketFD = createTCPIpv4Socket();

    char* serverIP = getServerIPAddress();
    printf("Server IP: %s\n", serverIP);

    struct sockaddr_in *serverAddress = createIPv4Address("", 2000);

    int result = bind(serverSocketFD, serverAddress, sizeof(struct sockaddr_in));

    if(result == 0)
        printf("Server started\n");
    else {
        printf("Failed to start server\n");
    }

    int listenResult = listen(serverSocketFD, 5);

    if(listenResult == -1){
        printf("Failed to listen\n");
        return 1;
    }


    startAcceptingIncomingConnections(serverSocketFD);



    shutdown(serverSocketFD, SHUT_RDWR);

    return 0;
}