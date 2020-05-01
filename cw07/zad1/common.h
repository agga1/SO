//
// Created by agness on 27.04.2020.
//

#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#define FTOK_PATH "0701"
#define PROJECT_ID 1

#define WAREHOUSE_SPACE 10
#define MAX_PACKAGE_SIZE 6000

#define CREATORS_COUNT 6
#define PACKERS_COUNT 4
#define SENDERS_COUNT 3

#define CREATORS_SEM 0
#define PACKERS_SEM 1
#define SENDERS_SEM 2
#define LOCK_MEM 3

typedef struct {
    int creators_idx;
    int packers_idx;
    int senders_idx;
    int packages[WAREHOUSE_SPACE];
} memory_t;

#endif //ZAD1_COMMON_H
