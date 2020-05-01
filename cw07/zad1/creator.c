//
// Created by agness on 27.04.2020.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>
#include <sys/shm.h>

#include "common.h"

int main() {
    srand(getpid());
    key_t key = ftok(FTOK_PATH, PROJECT_ID);

    int semGroupId = semget(key, 4, 0);
    int shMemId = shmget(key, sizeof(memory_t), 0);
    // operations to before creation
    struct sembuf lock_mem = {LOCK_MEM, -1, 0};
    struct sembuf decr_space = {CREATORS_SEM, -1, 0};
    struct sembuf ops_start[2] = {lock_mem, decr_space};
    // operations to do after creation
    struct sembuf unlock_mem = {LOCK_MEM, 1, 0};
    struct sembuf inc_to_prepare = {PACKERS_SEM, 1, 0};
    struct sembuf ops_end[2] = {unlock_mem, inc_to_prepare};

    while (1) {
        semop(semGroupId, ops_start, 2);
        memory_t *warehouse = shmat(shMemId, NULL, 0);

        // prepare new order
        int n = rand() % MAX_PACKAGE_SIZE/6 +1;
        int idx = warehouse->creators_idx;
        warehouse->packages[idx] = n;
        warehouse->creators_idx = (idx + 1) % WAREHOUSE_SPACE;

        // display info
        int to_prepare = semctl(semGroupId, PACKERS_SEM, GETVAL);
        int to_send = semctl(semGroupId, SENDERS_SEM, GETVAL);
        printf("(%d %lu) Dodalem liczbe: %d. ", getpid(), time(NULL), n);
        printf("Liczba zamowien do przygotowania: %d. ", to_prepare + 1);
        printf("Liczba zamowien do wyslania: %d\n", to_send);

        semop(semGroupId, ops_end, 2);
        shmdt(warehouse);

        sleep(1);
    }
}
