#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
bool running = true;
mqd_t serverQueID =-1;

struct Client{
    int queueId;
    char path[16];
    int peerId;
};
struct Client clients[MAX_CLIENTS + 1] = { 0 }; // client id begins from 1
int nextIdx = 1;
int connectedClients = 0;

void handleQueue(char *msg);
void handleNewClient(char *clientPath);
void handleStop(int clientId);
void handleList(int clientId);
void handleConnect(int id1, int id2);
void handleDisconnect(int clientId);
static void sigintHandler(int sig);

int main()
{
    // create server queue
    struct mq_attr attr;
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = MSGSIZE;
    serverQueID = mq_open(SERVER_QUEUE,  O_CREAT |O_EXCL| O_RDWR, 0666, &attr);
    if (serverQueID == -1) perrorAndQuit("serverQueID problem");
    printf("Created server queue with id %d.\n", serverQueID);

    catchSignal(SIGINT, sigintHandler);  // handle CTRL + C

    char request[MSGSIZE+2];
    while(running){
        memset(request, '\0', MSGSIZE+2);
        if (mq_receive(serverQueID, request, MSGSIZE+2, 0) < 0){
            printf("%s",request);
            perror("server: receiving message failed");
        }else{
            handleQueue(request);
        }

    }
    // destroy server queue
    closeAndQuit(serverQueID, SERVER_QUEUE);
    return 0;
}
void handleQueue(char *msg){
    /* msg like: mtype clientId args */
    if (msg == NULL) return;
    int mtype = atoi(strtok(msg, " "));
    int clientId = atoi(strtok(NULL, " "));
    char *args = strtok(NULL, "\0");
    printf("mtype: %d client %d args %s\n", mtype, clientId, args);
    switch(mtype){
        case STOP:
            handleStop(clientId);
            break;
        case DISCONNECT:
            handleDisconnect(clientId);
            break;
        case LIST:
            handleList(clientId);
            break;
        case CONNECT:
            handleConnect(clientId, atoi(args));
            break;
        case INIT:
            handleNewClient(args);
            break;
        default:
            puts("unknown signal");
    }
}
void send(mtype type, int clientID, char *msg ) {
    char message[MSGSIZE];
    snprintf(message, MSGSIZE, "%s", msg);
    printf("\nsent %s", message);
    if (mq_send(clients[clientID].queueId, message, MSGSIZE, type) == -1)
        perror("message not sent");
}
void handleStop(int clientId){
    send(STOP, clientId, "queue closing on server");
    clients[clientId].queueId = 0;
    clients[clientId].peerId = 0;
    usleep(1000);
    mq_close(clients[clientId].queueId);
    connectedClients -=1;
    puts("client removed");
}
void handleNewClient(char *clientPath){
    if(nextIdx > MAX_CLIENTS){
        puts("clients' list full");
        return;
    }
    clients[nextIdx].queueId = mq_open(clientPath, O_WRONLY);
    snprintf(clients[nextIdx].path, 16, "%s", clientPath);
    clients[nextIdx].peerId = 0;
    connectedClients += 1;
    nextIdx++;
    char id[10];
    sprintf(id, "%d", nextIdx -1);
    send(NEW_CLIENT, nextIdx-1, id);
}

void handleList(int clientId) {
    char text[MSG_LEN];
    strcpy(text, "active clients:\n");
    for(int i=0;i<MAX_CLIENTS+1;i++){
        if(clients[i].queueId>0) {
            char line[32];
            snprintf(line, 32, "clientId %d, available: %d", i, clients[i].peerId == 0);
            strcat(text, line);
            if(i == clientId) strcat(text, " [self]");
            strcat(text, "\n");
        }
    }
    send(LIST, clientId, text);
}
bool available(int id){
    if(id<0 || id> MAX_CLIENTS || clients[id].queueId ==0 || clients[id].peerId != 0)
        return false;
    return true;
}
void handleConnect(int id1, int id2) {
    printf("connecting [%d] and [%d] ", id1, id2);
    if(!available(id1)) {printf("client nr %d is unavailable", id1);
        return;}
    if(!available(id2)) {printf("client nr %d is unavailable\n", id2);
        return;}

    char msg1[MSG_LEN];
    char msg2[MSG_LEN];
    sprintf(msg1, "%s", clients[id2].path);
    sprintf(msg2, "%s", clients[id1].path);

    send(CONNECT, id1, msg1);
    send(CONNECT, id2, msg2);

    clients[id1].peerId = id2;
    clients[id2].peerId = id1;
}
void handleDisconnect(int clientId) {
    int id1 = clientId;
    int id2 = clients[id1].peerId;
    printf("disconnecting [%d] and [%d]\n", id1, id2);
    send(DISCONNECT, id1, "");
    send(DISCONNECT, id2, "");

    clients[id1].peerId = 0;
    clients[id2].peerId = 0;
}
static void sigintHandler (int signum){
    for (int i = 0; i < MAX_CLIENTS + 1; ++i) {
        if(clients[i].queueId > 0){
            handleStop(i);
        }
    }
    if(connectedClients != 0)
        printf("\nclosing with connected clients: %d left", connectedClients);

    closeAndQuit(serverQueID, SERVER_QUEUE);
}
