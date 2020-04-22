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
char clientPath[50];
int running = true;
int clientID = -1;
mqd_t serverQueId = -1;
mqd_t clientQueId = -1;

int peerQueue = -1;  // id of connected other client, if present
int inputProcess = -1; // proccess handling inputs from command line
int fd[2];  // pipe to send info (e.g. peerQueue nr) to inputProcess

void awaitClientId();
void send(int to, mtype type, char *text);
void handleInput(char *input); // input process
static void sigHandler(int sig); // input process
void handleQueue(struct Message* msg); // main process (handle incoming messages)

int main(){
    // turn off stdout buffering so messages are visible immediately
//    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) perrorAndQuit("cant change buffering mode");
    // get server queue
    serverQueId = mq_open(SERVER_QUEUE, O_RDWR);
    if (serverQueId == -1) perrorAndQuit("cant open serverQueue, make sure server running");
    // get client queue
    struct mq_attr attr;
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = MSGSIZE;
    snprintf(clientPath, 50, "/%d", getpid());
    clientQueId = mq_open(clientPath, O_CREAT | O_RDONLY | O_EXCL, 0666, &attr);
    if (clientQueId == -1) perrorAndQuit("clientQueueId problem");
    printf("Created client queue with id %d.\n", clientQueId);
    catchSignal(SIGINT, sigHandler);  // handle info that peerId is waiting

    send(serverQueId, INIT, clientPath);
    awaitClientId();
    closeAndQuit(clientQueId, clientPath);

}
ssize_t receive_command(char *msg, int *type){
    if (mq_receive(clientQueId, msg, MSGSIZE + 2, type) < 0){
        printf("%s", msg);
        perror("server: receiving message failed");
    }
    return 0;
}

void send(int to, mtype type, char *text) {
    char message[MSGSIZE];
    snprintf(message, MSGSIZE, "%d %d %s", type, clientID, text);
    if (mq_send(to, message, MSGSIZE, type) == -1)
        perror("message not sent");
}

void awaitClientId(){
    char msg[MSGSIZE+2];
    memset(msg, '\0', MSGSIZE+2);
    int type;
    receive_command(msg, &type);
    if(type == NEW_CLIENT){

    }
    printf("got %s", msg);
}
// inputProcess------------------
void handleInput(char *input){
    int cmd = -1;
    char *msg = parseTextOrCmd(input, &cmd);
    // cmd - command nr or -1, msg - text after command or whole text (if cmd=-1)
    if (cmd == -1){ // n
        if(peerQueue == -1)
            puts("Command unrecognized. \nto send message to server try command pattern:"
                 "\nCOMMAND some message (i.e CONNECT 2)\n"
                 "to start chat type: CONNECT friendId");
        else send(peerQueue, MESSAGE, msg);
        return;
    }
    switch (cmd){
        case CONNECT:
            send(serverQueId, CONNECT, msg);
            break;
        case DISCONNECT:
            send(serverQueId, DISCONNECT, msg);
            break;
        case STOP:
            send(serverQueId, STOP, msg);
            kill(getppid(), SIGKILL);
            closeAndQuit(clientQueId, clientPath);
            break;
        case LIST:
            send(serverQueId, LIST, msg);
            break;
        default:
            puts("unknown command");
    }
}

static void sigHandler (int signum){
    if(signum==SIGINT){
        send(serverQueId, STOP, "ok");
//        kill(getppid(), SIGINT);
        closeAndQuit(clientQueId, clientPath);
    }
    else if(signum == SIGUSR1){ // info that peerId is waiting
        read(fd[0], &peerQueue, sizeof(peerQueue));
        printf("peer Queue: %d\n", peerQueue);
    }
}
// -------main process--------------------------
void handleQueue(struct Message* msg){
    if (msg == NULL) return;
    switch(msg->mtype){
        case STOP:
            kill(inputProcess, SIGINT); // shut down from input process
            break;
        case DISCONNECT:
            peerQueue = -1;
            write(fd[1], &peerQueue, sizeof(peerQueue));
            kill(inputProcess, SIGUSR1);
            break;
        case CONNECT:
            peerQueue = atoi(msg->msg);
            write(fd[1], &peerQueue, sizeof(peerQueue));
            kill(inputProcess, SIGUSR1);
            if(peerQueue != -1)
                puts("you are in chat mode now. type DISCONNECT to disconnect");
            break;
        case MESSAGE:
            printf("[friend]:%s", msg->msg);
            break;
        case LIST:
            printf("%s", msg->msg);
            break;
        default:
            puts("unhandled signal");
    }
}

