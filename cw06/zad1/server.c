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
int main()
{
    // turn off stdout buffering so messages are visible immediately
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) perrorAndQuit("cant change buffering mode");
    // create server queue
    key_t serverKey =  ftok(FTOK_PATH, FTOK_ID);
    if (serverKey == -1)  perrorAndQuit("serverKey problem");
    int serverQueID = msgget(serverKey, IPC_CREAT | IPC_EXCL);
    if (serverQueID == -1) perrorAndQuit("serverQueID problem");
    printf("Created server queue with ID %d.\n", serverQueID);



    msgctl(serverQueID, IPC_RMID, NULL);  // destroy queue
    return 0;
}