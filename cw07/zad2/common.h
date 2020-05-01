//
// Created by agness on 01.05.2020.
//

#ifndef ZAD2_COMMON_H
#define ZAD2_COMMON_H

#define WAREHOUSE_SPACE 10
#define MAX_PACKAGE_SIZE 6000

#define CREATORS_CNT 8
#define PACKERS_CNT 4
#define SENDERS_CNT 3

#define CREATORS_SEM "/creators"
#define PACKERS_SEM "/packers"
#define SENDERS_SEM "/senders"
#define LOCK_MEM "/can_modify"

#define SH_MEM "/memory"

typedef struct {
    int creators_idx;
    int packers_idx;
    int senders_idx;
    int packages[WAREHOUSE_SPACE];
} memory_t;

#endif //ZAD2_COMMON_H
