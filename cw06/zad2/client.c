#include <stdio.h>
#include <stdlib.h>
# include "config.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
char clientPath[50];
int running = true;
int clientID = -1;
mqd_t serverQueue = -1;
mqd_t clientQueue = -1;

int peerQueue = -1;  // id of connected other client, if present

void awaitClientId();
void send(int to, mtype type, char *text);
void handleInput(char *input); // input process
static void sigHandler(int sig); // input process
static void notificationHandler(union sigval sv);
static void handleQueue(char *msg, unsigned int mtype); // main process (handle incoming messages)
void set_notification();
int main(){
    // get server queue
    serverQueue = mq_open(SERVER_QUEUE, O_RDWR);
    if (serverQueue == -1) perrorAndQuit("cant open serverQueue, make sure server running");
    // get client queue
    struct mq_attr attr;
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = MSGSIZE;
    snprintf(clientPath, 50, "/%d", getpid());
    clientQueue = mq_open(clientPath, O_CREAT | O_RDONLY | O_EXCL, 0666, &attr);
    if (clientQueue == -1) perrorAndQuit("clientQueueId problem");
    printf("Created client queue with id %d.\n", clientQueue);

    catchSignal(SIGINT, sigHandler);  // Ctrl+C
    catchSignal(SIGUSR1, notificationHandler); // get notification of received message
    // register client
    send(serverQueue, INIT, clientPath);
    awaitClientId();

    char input[MSG_LEN + 16]; // command + message textl
    set_notification();

    while(running){
        memset(input, '\0', MSG_LEN + 16);
        fgets(input, MSG_LEN+16, stdin);

        if (*input != '\0')
            handleInput(input);
    }
    closeAndQuit(clientQueue, clientPath);

}
void set_notification(){
    struct sigevent notification;
    notification.sigev_notify = SIGEV_SIGNAL;
    notification.sigev_signo = SIGUSR1;
    notification.sigev_value.sival_ptr = &clientQueue;
    if(mq_notify(clientQueue, &notification) == -1) perrorAndQuit("notification not set");
}
ssize_t receive_command(char *args, int *type){
    char message[MSGSIZE+2];
    if (mq_receive(clientQueue, message, MSGSIZE + 2, NULL) < 0){
        perror("server: receiving message failed");
    }
    *type = atoi(strtok(message, " "));
    strtok(NULL, " ");
    snprintf(args, MSGSIZE, "%s", strtok(NULL, "\0"));
    return 0;
}

void send(mqd_t to, mtype type, char *text) {
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
        clientID = atoi(msg);
    }
}
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
            send(serverQueue, CONNECT, msg);
            break;
        case DISCONNECT:
            send(serverQueue, DISCONNECT, msg);
            break;
        case STOP:
            send(serverQueue, STOP, msg);
            break;
        case LIST:
            send(serverQueue, LIST, msg);
            break;
        default:
            puts("unknown command");
    }
}

static void sigHandler (int signum){
    if(signum==SIGINT)
        send(serverQueue, STOP, "ok");
}
static void notificationHandler(union sigval sv) {
    char args[MSGSIZE + 2];
    memset(args, '\0', MSGSIZE + 2);
    int type;
    receive_command(args, &type);
    handleQueue(args, type);
    set_notification();
}

static void handleQueue(char *msg, unsigned int mtype){
    if (msg == NULL) return;
    switch(mtype){
        case STOP:
            // ready to be killed
            usleep(10000);
            if(peerQueue != -1) mq_close(peerQueue);  // cleanup peer queue
            closeAndQuit(clientQueue, clientPath);
            break;
        case DISCONNECT:
            mq_close(peerQueue);
            peerQueue = -1;
            break;
        case CONNECT:
            peerQueue = mq_open(msg, O_WRONLY);
            if(peerQueue != -1)
                puts("you are in chat mode now. type DISCONNECT to disconnect");
            else perror("connecting fail");

            break;
        case MESSAGE:
            printf("[friend]: %s", msg);
            break;
        case LIST:
            printf("%s", msg);
            break;
        default:
            puts("unhandled signal");
    }
}

