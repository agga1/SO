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
int peerQueue = -1;  // id of connected other client, if present
int serverQueId = -1;
int clientQueId;
int inputProcess = -1;
void registerSelf();
void send(int to, mtype type, char *text);
void awaitClientId();
void handleInput(char *input);
static void sigintHandler(int sig);
void handleQueue(struct Message* msg);


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
    printf("Created client queue with id %d.\n", clientQueId);


    registerSelf();
    awaitClientId(clientQueId);
    inputProcess = fork();
    if(inputProcess == 0){
        catchSignal(SIGINT, sigintHandler);  // handle CTRL + C
        char input[MSG_LEN + 16]; // command + message text; read from terminal
        while (running){
            memset(input, '\0', MSG_LEN + 16);
            if(peerQueue != -1) puts("some peer!");
            fgets(input, MSG_LEN+16, stdin);

            if (*input != '\0')
                handleInput(input);
        }
    }else{ // handle messages
        Message request;
        while(running) {
            if (msgrcv(clientQueId, &request, MSGSIZE, 0, 0) < 0)
                perrorAndQuit("server: receiving message failed");

            handleQueue(&request);
        }

        // destroy client queue
        closeAndQuit(clientQueId);
    }

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
    send(serverQueId, INIT, message);
}

void send(int to, mtype type, char *text) {
    Message message;
    message.clientId = clientID;
    message.mtype = type;
    snprintf(message.msg, MSG_LEN, "%s", text);
    if (msgsnd(to, &message, MSGSIZE, 0) == -1)
        printf("Message \"%s\" could not be send.\n", text);
    else
        puts("command send");
}

void awaitClientId(){
    Message message;
    receive_command(clientQueId, &message, NEW_CLIENT);
    if (running == false) // received SIGINT while waiting for NEW_CLIENT message
        return;
    clientID = message.clientId;
    printf("client received id %d\n", clientID);
}
// inputProcess------------------
void handleInput(char *input){
    char *cmd = strtok(input, " ");
    char *msg = strtok(NULL, "\n");
    if(msg == NULL){
        cmd[strlen(cmd)-1] = 0;
        msg = "";
    }
    int type = strToType(cmd);
    if (type == -1){ // chat mode
        puts("command pattern: COMMAND some message (i.e CONNECT 2)\n");
    }
    switch (type){
        case CONNECT:
            send(serverQueId, CONNECT, msg);
            // start subprocess for receiving messages
            break;
        case DISCONNECT:
            send(serverQueId, DISCONNECT, msg);
            break;
        case STOP:
            send(serverQueId, STOP, msg);
            kill(getppid(), SIGKILL);
            closeAndQuit(clientQueId);
            break;
        case LIST:
            send(serverQueId, LIST, msg);
            // or list here?
            break;
        default:
            puts("unknown command");
    }
}

static void sigintHandler (int signum){
    send(serverQueId, STOP, "");
    kill(getppid(), SIGINT);
    closeAndQuit(clientQueId);
}
// -------main process--------------------------
void handleQueue(struct Message* msg){
    if (msg == NULL) return;
    switch(msg->mtype){
        case STOP:
            kill(inputProcess, SIGINT); // shut down from input process
            break;
        case DISCONNECT:
            break;
        case CONNECT:
            puts("received queue!");
            peerQueue = atoi(msg->msg);
            break;
        case MESSAGE:
            puts("received message!");
            printf("%s", msg->msg);
            break;
        default:
            puts("unhandled signal");
    }
}
