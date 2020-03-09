//
// Created by Agnieszka on 07/03/2020.
//
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>
#include<sys/times.h>
#include <time.h>
//#include <bits/time.h>
#include <string.h>
#include <error.h>
#include <stdbool.h>
#include "library.h"
#define N 7

void append_file(FILE* result_file, char* data){
    fprintf(result_file, "%s\n", data);
}
void write_time(FILE *result_file, struct timespec *start_time, struct timespec *end_time, struct rusage *start_usage,
                struct rusage *end_usage){
    fprintf(result_file, "REAL_TIME: %ldns\n", end_time->tv_nsec - start_time->tv_nsec);
    fprintf(result_file, "USER_TIME: %ldµs\n", end_usage->ru_utime.tv_usec - start_usage->ru_utime.tv_usec);
    fprintf(result_file, "SYSTEM_TIME: %ldµs\n", end_usage->ru_stime.tv_usec - start_usage->ru_stime.tv_usec);
}
bool in(char* el, char ** str_array, int ar_len){
    for(int i = 0; i < ar_len; i++) {
        if(strcmp(str_array[i], el) == 0)
            return true;
    }
    return false;
}
double calculate_time(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}
int main(int argc, char **argv){
    char *commands[N]= {"create_table", "compare_pairs", "remove_block", "remove_operation", "save_block", "start_time", "clear_file"};

    struct tms **tms_time = malloc(6 * sizeof(struct tms *));
    clock_t real_time[6];
    for (int i = 0; i < 6; i++) {
        tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    }

//    printf("running");
    //// time measurement
    const char* cwd = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/ex2/txt_files/";
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s%s", cwd, "raport2.txt");

    FILE *result_file;
    int nr=1;
    if(strcmp(argv[nr], commands[6])==0) {result_file = fopen(path, "w"); nr++;}
    else result_file = fopen(path, "a");

    struct rusage *start_usage = calloc(1, sizeof * start_usage);
    struct rusage *end_usage = calloc(1, sizeof * end_usage);
    struct timespec *start_time = calloc(1, sizeof *start_time);
    struct timespec *end_time = calloc(1, sizeof *end_time);


    append_file(result_file, "\texecuted operations:");
    clock_gettime(0, start_time); // if "start" not defined, measure all by default
    getrusage(RUSAGE_SELF, start_usage);
    bool count_time = false;

    if(strcmp(argv[nr], commands[0])!=0) {
        printf("command chain must start with %s!", commands[0]);
        return 1;
    }

    int size = atoi(argv[++nr]);
    struct main_array *ma = main_array_new(size);

    for(int i=++nr; i<argc; i++){
        if(strcmp(argv[i], commands[1])==0){ // compare pairs
            int start = ++i;
            int length = 0;
            while (i < argc && !in(argv[i], commands, N)){
                length++;
                i++;
            }
            char **pairs = calloc((size_t) length, sizeof(char*));
            for(int j=start; j<i;j++) pairs[j-start]=argv[j];

            compare_to_tmp_file(pairs, length);

            char cmd[256];
            if(length<=5) snprintf(cmd, 256, "%s size small", commands[1]);
            else if(length<15) snprintf(cmd, 256, "%s size medium", commands[1]);
            else snprintf(cmd, 256, "%s size large", commands[1]);
            if(count_time) append_file(result_file, cmd);

            i--;
        }
        else if(strcmp(argv[i], commands[2])==0){ // remove block
            remove_block(atoi(argv[++i]), ma);
            if(count_time) append_file(result_file, commands[2]);

        }
        else if(strcmp(argv[i], commands[3])==0){ // remove operation
            int block_nr = atoi(argv[++i]);
            int ed_op_nr = atoi(argv[++i]);
            remove_ed_op(block_nr, ed_op_nr, ma);
            if(count_time) append_file(result_file, commands[3]);
        }
        else if(strcmp(argv[i], commands[4])==0){ // save block
            int block_nr = atoi(argv[++i]);
            save_block(block_nr, ma);
            if(count_time) append_file(result_file, commands[4]);
        }
        else if(strcmp(argv[i], commands[5])==0){
            clock_gettime(0, start_time);
            getrusage(RUSAGE_SELF, start_usage);
            count_time=true;
        }
    }

    clock_gettime(0, end_time);
    getrusage(RUSAGE_SELF, end_usage);

    write_time(result_file, start_time, end_time, start_usage, end_usage);
    fclose(result_file);
    return 0;
}


