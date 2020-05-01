//
// Created by agness on 27.04.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

int main() {
    key_t key = ftok(FTOK_PATH, PROJECT_ID);

    int semaphores = semget(key, 4, 0);
    int memory = shmget(key, sizeof(memory_t), 0);

    struct sembuf lock_memory = {LOCK_MEM, -1, 0};
    struct sembuf decr_to_send = {SENDERS_SEM, -1, 0};
    struct sembuf ops_start[2] = {lock_memory, decr_to_send};

    struct sembuf unlock_memory = {LOCK_MEM, 1, 0};
    struct sembuf inc_space = {CREATORS_SEM, 1, 0};
    struct sembuf ops_end[2] = {unlock_memory, inc_space};

    while (1) {
        semop(semaphores, ops_start, 2);

        memory_t *warehouse = shmat(memory, NULL, 0);
        int idx = warehouse->senders_idx;
        warehouse->senders_idx = (idx + 1) % WAREHOUSE_SPACE;
        warehouse->packages[idx] *= 3;
        int n = warehouse->packages[idx];

        // display info
        int to_prepare = semctl(semaphores, PACKERS_SEM, GETVAL);
        int to_send = semctl(semaphores, SENDERS_SEM, GETVAL);
        printf("(%d %lu) Wyslalem zamowienie o wielkosci %d. ", getpid(),
               time(NULL), n);
        printf("Liczba zamowien do przygotowania: %d. ", to_prepare);
        printf("Liczba zamowien do wyslania: %d\n", to_send);

        semop(semaphores, ops_end, 2);
        shmdt(warehouse);

        sleep(1);
    }
}
