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
    int shMemId = shmget(key, sizeof(memory_t), 0);

    struct sembuf lock_memory = {LOCK_MEM, -1, 0};
    struct sembuf decrement_created = {CREATED_INDEX, -1, 0};
    struct sembuf ops_start[2] = {lock_memory, decrement_created};

    struct sembuf unlock_memory = {LOCK_MEM, 1, 0};
    struct sembuf increment_packed = {PACKED_INDEX, 1, 0};
    struct sembuf ops_end[2] = {unlock_memory, increment_packed};

    while (1) {
        semop(semaphores, ops_start, 2);

        memory_t *warehouse = shmat(shMemId, NULL, 0);
        //find order to pack
        int idx = warehouse->index;
        while (warehouse->packages[idx].status != CREATED) {
            idx = (idx + 1) % WAREHOUSE_SPACE;
        }

        warehouse->packages[idx].status = PACKED;
        warehouse->packages[idx].value *= 2;
        int n = warehouse->packages[idx].value;

        int created_count = semctl(semaphores, CREATED_INDEX, GETVAL);
        int packed_count = semctl(semaphores, PACKED_INDEX, GETVAL);

        printf("(%d %lu) Przygotowalem zamowienie o wielkosci %d. ", getpid(),
               time(NULL), n);
        printf("Liczba paczek do przygotowania: %d. ", created_count);
        printf("Liczba paczek do wyslania: %d\n", packed_count + 1);

        semop(semaphores, ops_end, 2);
        shmdt(warehouse);

        sleep(1);
    }
}
