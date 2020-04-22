#ifndef ZAD1_CONFIG_H
#define ZAD1_CONFIG_H

#define FTOK_PATH  getenv("HOME")
#define FTOK_ID  17
#define MAX_CLIENTS 16
#define MSG_LEN 512
# define MSGSIZE MSG_LEN+sizeof(int)

#include <string.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdbool.h>


typedef enum mtype
{
    INIT = 1,
    LIST = 2,
    CONNECT = 3,
    DISCONNECT = 4,
    STOP = 5,
    NEW_CLIENT = 6,
    MESSAGE = 7
} mtype;  // TODO rename


typedef struct Message {
    long mtype;
    int clientId;
    char msg[MSG_LEN];
} Message;

int strToType(char *str){ // converts commands (only those available for clients) to enum mtype
    if(strcmp(str, "INIT")==0) return INIT;
    if(strcmp(str, "LIST")==0) return LIST;
    if(strcmp(str, "CONNECT")==0) return CONNECT;
    if(strcmp(str, "DISCONNECT")==0) return DISCONNECT;
    if(strcmp(str, "STOP")==0) return STOP;
    return -1;
}
void closeAndQuit(int queueID){
    puts("closing queue and shutting down...");
    msgctl(queueID, IPC_RMID, NULL);
    exit(0);
}
void perrorAndQuit(char *msg){
    perror(msg);
    exit(1);
}

void catchSignal(int sig, void (*func)(int)){
    struct sigaction act;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(sig, &act, NULL);
}
char* parseTextOrCmd(char *input, int *cmd){
    char *inputCp = calloc(MSG_LEN+16, sizeof(char));
    strncpy(inputCp, input, MSG_LEN+16);
    char *cmdstr = strtok(input, " ");
    char *msg = strtok(NULL, "\0");
    if(msg == NULL){ // one word only
        cmdstr[strlen(cmdstr)-1] = 0; // cut \n
        msg = "";               // empty msg
    }
    // more than 1 word
    int type = strToType(cmdstr);
    if (type == -1){
        *cmd = -1;
        return inputCp;
    }
    *cmd = type;
    return msg;
}
#endif //ZAD1_CONFIG_H
