#include <stdio.h>
#include <stdlib.h>
# include "config.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
bool running = true;
int serverQueID = -1;
typedef struct Client{
    int queueId;
    pid_t pid;
    bool available;
} client;
client clients[MAX_CLIENTS + 1]; // client id begins from 1
int nextIdx = 1;
int connectedClients = 0;

void handleQueue(struct Message *msg);
void handleStop(Message *msg);
void handleNewClient(struct Message *msg);
static void sigintHandler(int sig);

int main()
{
    // turn off stdout buffering so messages are visible immediately
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) perrorAndQuit("cant change buffering mode");
    // create server queue
    key_t serverKey =  ftok(FTOK_PATH, FTOK_ID);
    if (serverKey == -1)  perrorAndQuit("serverKey problem");
    serverQueID = msgget(serverKey, IPC_CREAT | 0666);
    if (serverQueID == -1) perrorAndQuit("serverQueID problem");
    printf("Created server queue with id %d.\n", serverQueID);

    catchSignal(SIGINT, sigintHandler);  // handle CTRL + C

    memset(clients, 0, (MAX_CLIENTS+1) * sizeof(client));

    Message request;
    while(running){
        // handling higher priority signals
        if(msgrcv(serverQueID, &request, MSGSIZE, STOP, MSG_NOERROR | IPC_NOWAIT) != -1){
            handleStop(&request);
            continue;
        }
        if(msgrcv(serverQueID, &request, MSGSIZE, DISCONNECT, MSG_NOERROR | IPC_NOWAIT) != -1){
//            disconnect();
            puts("DISCONNECT requested");
            continue;
        }
        if(msgrcv(serverQueID, &request, MSGSIZE, LIST, MSG_NOERROR | IPC_NOWAIT) != -1){
//            list();
            puts("LIST requested");
            continue;
        }

        // waiting for any signal
        if (msgrcv(serverQueID, &request, MSGSIZE, 0, 0) < 0)
            perrorAndQuit("server: receiving message failed");

        handleQueue(&request);
    }
    // destroy server queue
    msgctl(serverQueID, IPC_RMID, NULL);  // destroy queue

    return 0;
}
void handleQueue(struct Message* msg){
    if (msg == NULL) return;
    switch(msg->mtype){
        case STOP:
            handleStop(msg);
            break;
        case DISCONNECT:
            break;
        case LIST:
            break;
        case INIT:
            handleNewClient(msg);
            break;
        default:
            puts("unknown signal");
    }
}
void send(mtype type, int clientID, char *msg ) {
    Message message;
    message.clientId = clientID;
    message.mtype = type;
    snprintf(message.msg, MSG_LEN, "%s", msg);
    if (msgsnd(clients[clientID].queueId, &message, MSGSIZE, 0) == -1)
        printf("Message \"%s\" could not be send.\n", msg);
    else
        puts("message sent");
}
void handleStop(Message *msg){
    clients[msg->clientId].pid = 0;
    clients[msg->clientId].queueId = 0;
    clients[msg->clientId].available = true;
    connectedClients -=1;
    puts("client removed");
}

void handleNewClient(struct Message *msg){
    printf("data %s", msg->msg);
    if(nextIdx > MAX_CLIENTS){
        puts("clients' list full");
        return;
    }
    pid_t clientPid = (pid_t) strtol(strtok(msg->msg, " "), NULL, 10);
    int clientQueueId = (int) strtol(strtok(NULL, "\0"), NULL, 10);
    clients[nextIdx].queueId = clientQueueId;
    clients[nextIdx].pid = clientPid;
    clients[nextIdx].available = true;
    connectedClients += 1;
    nextIdx++; // TODO search for new spot

    send(NEW_CLIENT, nextIdx-1, "");
}
static void sigintHandler (int signum){
    for (int i = 0; i < MAX_CLIENTS + 1; ++i) {
        if(clients[i].pid > 0){
            kill(clients[i].pid, SIGINT);  // shut down all the clients and wait for confirmation
            Message request;
            msgrcv(serverQueID, &request, MSGSIZE, STOP, 0);
            handleStop(&request);
        }
    }
    if(connectedClients != 0)
        printf("closing with connected clients: %d left", connectedClients);
    closeAndQuit(serverQueID);
}
