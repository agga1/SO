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
    if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
        printf("Error: buffering mode could not be changed!\n");
        exit(1);
    }
    key_t serverKey =  ftok(FTOK_PATH, FTOK_ID);
    printf("%i", serverKey);
    int serverQueID = msgget(serverKey, IPC_CREAT | 0666);
    if (serverQueID == -1) {
        perror("well,: ");
        exit(1);
    }
    printf("Created server queue with ID %d.\n", serverQueID);
    return 0;
}