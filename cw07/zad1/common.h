//
// Created by agness on 27.04.2020.
//

#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#define FTOK_PATH "0701"
#define PROJECT_ID 1

#define WAREHOUSE_SPACE 10
#define MAX_PACKAGE_SIZE 6000

#define CREATORS_COUNT 7
#define PACKERS_COUNT 5
#define SENDERS_COUNT 3

typedef enum { CREATED, PACKED, SENT } package_status;
typedef struct {
    package_status status;
    int value;
} package_t;

#define SPACE_INDEX 0
#define CREATED_INDEX 1
#define PACKED_INDEX 2
#define LOCK_MEM 3

typedef struct {
    int index;
    int size;
    package_t packages[WAREHOUSE_SPACE];
} memory_t;

#endif //ZAD1_COMMON_H
