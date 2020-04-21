#ifndef ZAD1_CONFIG_H
#define ZAD1_CONFIG_H

#define FTOK_PATH  "file"
#define FTOK_ID  17
#define MAX_CLIENTS 16
#define MSG_LEN 256
# define MSGSIZE MSG_LEN+sizeof(int)

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


#endif //ZAD1_CONFIG_H
