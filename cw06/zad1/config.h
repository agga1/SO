#ifndef ZAD1_CONFIG_H
#define ZAD1_CONFIG_H

#define FTOK_PATH  "file2"
#define FTOK_ID  17
#define MAX_CLIENTS 16
#define MSG_LEN 256
# define MSGSIZE MSG_LEN+sizeof(int)

#include <string.h>
void perrorAndQuit(char *msg){
    perror(msg);
    exit(1);
}

typedef enum mtype
{
    INIT = 1,
    LIST = 2,
    CONNECT = 3,
    DISCONNECT =4,
    STOP = 5,
    NEW_CLIENT = 6,
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
#endif //ZAD1_CONFIG_H
