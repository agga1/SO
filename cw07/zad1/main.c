#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common.h"

int semGroupId = -1;
int sharedMem = -1;

pid_t workers[CREATORS_COUNT + PACKERS_COUNT + SENDERS_COUNT];

void sigtermHandler() {
    for (int i = 0; i < CREATORS_COUNT + PACKERS_COUNT + SENDERS_COUNT; i++) {
        kill(workers[i], SIGTERM);
    }
}

int main() {
    signal(SIGINT, sigtermHandler);
    key_t key = ftok(FTOK_PATH, PROJECT_ID);

    semGroupId = semget(key, 4, IPC_CREAT | 0666);
    semctl(semGroupId, CREATORS_SEM, SETVAL, WAREHOUSE_SPACE);
    semctl(semGroupId, PACKERS_SEM, SETVAL, 0);
    semctl(semGroupId, SENDERS_SEM, SETVAL, 0);
    semctl(semGroupId, LOCK_MEM, SETVAL, 1);

    sharedMem = shmget(key, sizeof(memory_t), IPC_CREAT | 0666);
    memory_t* warehouse = shmat(sharedMem, NULL, 0);
    warehouse->creators_idx = 0;
    warehouse->packers_idx = 0;
    warehouse->senders_idx = 0;
    for (int i = 0; i < WAREHOUSE_SPACE; i++) { // TODO memset?
        warehouse->packages[i] = 0;
    }
    shmdt(warehouse);
    int j = 0;
    for (int i = 0; i < CREATORS_COUNT; i++) {
        workers[j] = fork();
        if (workers[j] == 0) {
            execlp("./creator", "./creator", NULL);
            return 1;
        }
        j++;
    }

    for (int i = 0; i < PACKERS_COUNT; i++) {
        workers[j] = fork();
        if (workers[j] == 0) {
            execlp("./packer", "./packer", NULL);
            perror("test");
            return 1;
        }
        j++;
    }

    for (int i = 0; i < SENDERS_COUNT; i++) {
        workers[j] = fork();
        if (workers[j] == 0) {
            execlp("./sender", "./sender", NULL);
            return 1;
        }
        j++;
    }

    for (int i = 0; i < CREATORS_COUNT + PACKERS_COUNT + SENDERS_COUNT; i++) {
        wait(0);
    }

    if (semGroupId != -1) {
        semctl(semGroupId, 0, IPC_RMID);
    }
    if (sharedMem != -1) {
        shmctl(sharedMem, IPC_RMID, NULL);
    }

    return 0;
}