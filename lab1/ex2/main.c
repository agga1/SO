//
// Created by Agnieszka on 07/03/2020.
//
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>
#include <bits/time.h>
#include <string.h>
#include <error.h>
#include "library.h"

void writeResult(FILE* result_file, struct timespec * start_time, struct timespec * end_time, struct rusage *start_usage, struct rusage *end_usage){
    fprintf(result_file, "REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    fprintf(result_file, "USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    fprintf(result_file, "SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);
}

int main(int argc, char **argv){

    if(strcmp(argv[1], "create_table")!=0) {
        printf("First argument must be create_table!");
        return 1;
    }
    int size = atoi(argv[2]);
    struct main_array *ma = main_array_new(size);

    for(int i=3; i<argc; i++){
        if(strcmp(argv[i], "compare_pairs")==0){
            int start = ++i;
            int length = 0;
            while (i < argc && (strcmp(argv[i], "save_block")!=0 || strcmp(argv[i], "compare_pairs")!=0 || strcmp(argv[i], "remove_block")!=0 || strcmp(argv[i], "remove_operation")!=0)){
                length++;
                i++;
            }
            char *seq = calloc((size_t) length, sizeof(char*));
            strncpy(seq, argv[start], length* sizeof(char));
            char **pairs = seq_to_pair_array(seq, length);
            // start time
            compare_to_tmp_file(pairs, length);
            // end time
        }
        else if(strcmp(argv[i], "remove_block")==0){
        }
        else if(strcmp(argv[i], "save_block")==0){
        }
        else if(strcmp(argv[i], "remove_operation")==0){
            int block_to_delete_from = ++i;
            int operation_to_delete = ++i;
        }
    }
    //// time measurement
    const char* cwd = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/ex2/txt_files/";
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s%s", cwd, "raport2.txt");
    FILE *result_file = fopen(path, "a");
    struct rusage *start_usage = calloc(1, sizeof * start_usage);
    struct rusage *end_usage = calloc(1, sizeof * end_usage);
    struct timespec *start_time = calloc(1, sizeof *start_time);
    struct timespec *end_time = calloc(1, sizeof *end_time);
    clock_gettime(CLOCK_REALTIME, start_time);
    getrusage(RUSAGE_SELF, start_usage);
    clock_gettime(CLOCK_REALTIME, end_time);
    getrusage(RUSAGE_SELF, end_usage);
    writeResult(result_file, start_time, end_time, start_usage, end_usage);
    fclose(result_file);
    return 0;
}


