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
int clientID = -1;
int serverQueId = -1;
int clientQueId = -1;

int peerQueue = -1;  // id of connected other client, if present
int inputProcess = -1; // proccess handling inputs from command line
int fd[2];  // pipe to send info (e.g. peerQueue nr) to inputProcess

void registerSelf();
void awaitClientId();
void send(int to, mtype type, char *text);
void handleInput(char *input); // input process
static void sigHandler(int sig); // input process
void handleQueue(struct Message* msg); // main process (handle incoming messages)

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
    // start work - create inputProcess and listen for:
    // incoming console input (child)/ sys V messages(parent)
    pipe(fd);
    inputProcess = fork();
    if(inputProcess == 0){
        close(fd[1]);
        catchSignal(SIGINT, sigHandler);  // handle CTRL + C
        catchSignal(SIGUSR1, sigHandler);  // handle info that peerId is waiting
        char input[MSG_LEN + 16]; // command + message text; read from terminal
        while (running){
            memset(input, '\0', MSG_LEN + 16);
            fgets(input, MSG_LEN+16, stdin);

            if (*input != '\0')
                handleInput(input);
        }
    }else{ // handle messages
        close(fd[0]);

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
    snprintf(message, MSG_LEN, "%d", clientQueId);
    send(serverQueId, INIT, message);
}

void send(int to, mtype type, char *text) {
    Message message;
    message.clientId = clientID;
    message.mtype = type;
    snprintf(message.msg, MSG_LEN, "%s", text);
    if (msgsnd(to, &message, MSGSIZE, 0) == -1)
        printf("Message \"%s\" could not be send.\n", text);
}

void awaitClientId(){
    Message message;
    receive_command(clientQueId, &message, NEW_CLIENT);
    clientID = message.clientId;
    printf("client received id %d\n", clientID);
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
            closeAndQuit(clientQueId);
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
        send(serverQueId, STOP, "");
        kill(getppid(), SIGINT);
        closeAndQuit(clientQueId);
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
