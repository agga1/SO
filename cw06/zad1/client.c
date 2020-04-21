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


int running = true;
int clientID = 0;  // 0 means unregistered client
int serverQueId = -1;
int clientQueId;

void registerSelf();
void send(mtype type, char *msg);
void awaitClientId();

int main(){
    // turn off stdout buffering so messages are visible immediately
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) perrorAndQuit("cant change buffering mode");
    // get server queue
    key_t serverKey =  ftok(FTOK_PATH, FTOK_ID);
    if(serverKey==-1) perrorAndQuit("serverKey problem");
    serverQueId = msgget(serverKey, 0);
    if (serverQueId == -1) perrorAndQuit("cant open serverQueue, make sure servver running");
    // get client queue
    clientQueId = msgget(IPC_PRIVATE, 0666);
    if (clientQueId == -1) perrorAndQuit("ClientQueId problem");
    printf("Created client queue with ID %d.\n", clientQueId);

    registerSelf();
    awaitClientId(clientQueId);

    // destroy client queue
    msgctl(clientQueId, IPC_RMID, NULL);
    return 0;
}
ssize_t receive_command(int queue_ID, Message *message_buffer, long msgtype){
    ssize_t received = msgrcv(queue_ID, message_buffer, MSGSIZE, msgtype, 0);
    if (received == -1){
        if (errno == EINTR) return -1; // During waiting for a message SIGINT occurred
        puts("message not received properly");
    }
    return received;
}
void registerSelf() {
    char message[MSG_LEN];
    snprintf(message, MSG_LEN, "%d %d",  getpid(), clientQueId);
    send(INIT, message);
}

void send(mtype type, char *msg) {
    Message message;
    message.clientId = clientID;
    message.mtype = type;
    snprintf(message.msg, MSG_LEN, "%s", msg);
    if (msgsnd(serverQueId, &message, MSGSIZE, 0) == -1)
        printf("Message \"%s\" could not be send.\n", msg);
    else
        puts("command send");
}

void awaitClientId(){
    Message message;
    receive_command(clientQueId, &message, NEW_CLIENT);
    if (running == false) // received SIGINT while waiting for NEW_CLIENT message
        return;
    printf("Client ready for work.\n");
    clientID = (int) strtol(message.msg, NULL, 10);
}

