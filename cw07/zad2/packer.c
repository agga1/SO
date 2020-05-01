//
// Created by agness on 01.05.2020.
//
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

sem_t* packers;
sem_t* senders;
sem_t* can_modify;

void sigterm_handler() {
    sem_close(packers);
    sem_close(senders);
    sem_close(can_modify);

    shm_unlink(SH_MEM);
    exit(0);
}

int main() {
    signal(SIGTERM, sigterm_handler);

    packers = sem_open(PACKERS_SEM, O_RDWR, 0666);
    senders = sem_open(SENDERS_SEM, O_RDWR, 0666);
    can_modify = sem_open(LOCK_MEM, O_RDWR, 0666);

    int memory = shm_open(SH_MEM, O_RDWR, 0666);

    while (1) {
        sem_wait(packers);
        sem_wait(can_modify);

        memory_t* warehouse = mmap(NULL, sizeof(memory_t), PROT_WRITE, MAP_SHARED, memory, 0);

        // prepare package
        int idx = warehouse->packers_idx;
        warehouse->packers_idx = (idx + 1) % WAREHOUSE_SPACE;
        warehouse->packages[idx] *= 2;
        int n = warehouse->packages[idx];

        // display info
        int to_prepare, to_send;
        sem_getvalue(packers, &to_prepare);
        sem_getvalue(senders, &to_send);
        printf("(%d %lu) Przygotowalem zamowienie o wielkosci %d. ", getpid(), time(NULL), n);
        printf("Liczba zamowien do przygotowania: %d. ", to_prepare);
        printf("Liczba zamowien do wyslania: %d\n", to_send + 1);

        sem_post(senders);
        sem_post(can_modify);

        munmap(warehouse, sizeof(memory_t));
        sleep(1);
    }
}

