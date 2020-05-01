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
    srand(getpid());
    key_t key = ftok(FTOK_PATH, PROJECT_ID);

    int semGroupId = semget(key, 4, 0);
    int shMemId = shmget(key, sizeof(memory_t), 0);

    struct sembuf lock_memory = {LOCK_MEM, -1, 0};
    struct sembuf decrement_space = {SPACE_INDEX, -1, 0};
    struct sembuf ops_start[2] = {lock_memory, decrement_space};

    struct sembuf unlock_memory = {LOCK_MEM, 1, 0};
    struct sembuf increment_created = {CREATED_INDEX, 1, 0};
    struct sembuf ops_end[2] = {unlock_memory, increment_created};

    while (1) {
        semop(semGroupId, ops_start, 2);

        int n = rand() % MAX_PACKAGE_SIZE/6 +1;

        memory_t *warehouse = shmat(shMemId, NULL, 0);

        int index;
        if (warehouse->index == -1) {
            warehouse->index = 0;
            index = 0;
        } else {
            index = (warehouse->index + warehouse->size) % WAREHOUSE_SPACE;
        }

        warehouse->packages[index].status = CREATED;
        warehouse->packages[index].value = n;
        warehouse->size++;

        int created_count = semctl(semGroupId, CREATED_INDEX, GETVAL);
        int packed_count = semctl(semGroupId, PACKED_INDEX, GETVAL);

        printf("(%d %lu) Dostalem liczbe %d. ", getpid(), time(NULL), n);
        printf("Liczba paczek do przygotowania: %d. ", created_count + 1);
        printf("Liczba paczek do wyslania: %d\n", packed_count);

        semop(semGroupId, ops_end, 2);
        shmdt(warehouse);

        sleep(1);
    }
}
