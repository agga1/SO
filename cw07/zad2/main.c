#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"

pid_t workers[CREATORS_CNT + PACKERS_CNT + SENDERS_CNT];
void sigint_handler() {
    for (int i = 0; i < CREATORS_CNT + PACKERS_CNT + SENDERS_CNT; i++)
        kill(workers[i], SIGTERM);
}

int main() {
    signal(SIGINT, sigint_handler);

    sem_t *creators = sem_open(CREATORS_SEM, O_CREAT | O_RDWR, 0666, WAREHOUSE_SPACE);
    sem_t *packers = sem_open(PACKERS_SEM, O_CREAT | O_RDWR, 0666, 0);
    sem_t *senders = sem_open(SENDERS_SEM, O_CREAT | O_RDWR, 0666, 0);
    sem_t *can_modify = sem_open(LOCK_MEM, O_CREAT | O_RDWR, 0666, 1);

    int memory = shm_open(SH_MEM, O_CREAT | O_RDWR, 0666);
    ftruncate(memory, sizeof(memory_t));

    memory_t *warehouse = mmap(NULL, sizeof(memory_t), PROT_WRITE, MAP_SHARED, memory, 0);
    warehouse->creators_idx = 0;
    warehouse->packers_idx = 0;
    warehouse->senders_idx = 0;
    for (int i = 0; i < WAREHOUSE_SPACE; i++) warehouse->packages[i] = 0;

    munmap(warehouse, sizeof(memory_t));

    int j = 0;
    for (int i = 0; i < CREATORS_CNT; i++) {
        workers[j] = fork();
        if (workers[j] == 0) {
            execlp("./creator", "./creator", NULL);
            return 1;
        }
        j++;
    }

    for (int i = 0; i < PACKERS_CNT; i++) {
        workers[j] = fork();
        if (workers[j] == 0) {
            execlp("./packer", "./packer", NULL);
            return 1;
        }
        j++;
    }

    for (int i = 0; i < SENDERS_CNT; i++) {
        workers[j] = fork();
        if (workers[j] == 0) {
            execlp("./sender", "./sender", NULL);
            return 1;
        }
        j++;
    }

    for (int i = 0; i < CREATORS_CNT + PACKERS_CNT + SENDERS_CNT; i++) {
        wait(0);
    }
    sem_close(creators);
    sem_close(packers);
    sem_close(senders);
    sem_close(can_modify);

    sem_unlink(CREATORS_SEM);
    sem_unlink(PACKERS_SEM);
    sem_unlink(SENDERS_SEM);
    sem_unlink(LOCK_MEM);

    shm_unlink(SH_MEM);
    return 0;
}
