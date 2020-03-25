#define _XOPEN_SOURCE 500
#define MODE_JOINT 1
#define MODE_DISJOINT 2
#define MAX_COL 1000 // maximum nr of columns in any result matrix
#include <linux/limits.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "matrix_manage.c"
#include <sys/resource.h>
const char* out_folder = "tmp";

void create_out_folder();

void print_stats(struct rusage *start_t, struct rusage *end_t);

//// calculating column nr @col of output matrix to separate file
void calc_separate_col(struct matrix *a, struct matrix *b, int col, int pair_index) {
    char* filename = calloc(20, sizeof(char));
    sprintf(filename, "%s/part%02d%04d", out_folder, pair_index, col);
    FILE* part_file = fopen(filename, "w+");

    for (int a_row = 0; a_row < a->row_nr; a_row++) {
        int result = 0;
        for (int a_col = 0; a_col < a->col_nr; a_col++)
            result += a->mx[a_row][a_col] * b->mx[a_col][col];
        if(a_row == a->row_nr -1 ) fprintf(part_file, "%d", result);
        else fprintf(part_file, "%d\n", result);
    }
    fclose(part_file);
}
//// calculating column nr @col in output matrix
void calc_col_in_mx(struct matrix *a, struct matrix *b, int col, char *c_file) {
    FILE* file = fopen(c_file, "r+");
    int fd = fileno(file);
    flock(fd, LOCK_EX);
    struct matrix *c = load_mx(c_file);

    for (int a_row = 0; a_row < a->row_nr; a_row++) {
        int result = 0;
        for (int a_col = 0; a_col < a->col_nr; a_col++) {
            result += a->mx[a_row][a_col] * b->mx[a_col][col];
        }
        c->mx[a_row][col] = result;
    }
    write_mx_to_file(file, c);
    flock(fd, LOCK_UN);
    fclose(file);
}
int* get_task(int total_pairs) {
    int *pair_and_col = calloc(2, sizeof(int));
    pair_and_col[0]=pair_and_col[1]=-1;

    for(int pair_idx=0; pair_idx < total_pairs; pair_idx++){
        char* filename = calloc(100, sizeof(char));
        sprintf(filename, "%s/tasks%03d",out_folder, pair_idx);
        FILE* tasksF = fopen(filename, "r+");
        //// get file descriptor and lock file
        int fd = fileno(tasksF);
        flock(fd, LOCK_EX);

        char* tasks = calloc(MAX_COL, sizeof(char));
        fseek(tasksF, 0, SEEK_SET);
        fread(tasks, 1, MAX_COL, tasksF);
        char* get_undone = strchr(tasks, '0');

        if (get_undone != NULL) {
            int to_do_idx = (int) (get_undone - tasks);
            size_t total_tasks = (strchr(tasks, '\0') - tasks);
            tasks[to_do_idx] = '1';
            //// update done tasks
            fseek(tasksF, 0, SEEK_SET);
            fwrite(tasks, 1, total_tasks, tasksF);
            pair_and_col[0] = pair_idx;
            pair_and_col[1] = to_do_idx;

            flock(fd, LOCK_UN);
            fclose(tasksF);
            return pair_and_col;
        }
        flock(fd, LOCK_UN);
        fclose(tasksF);
    }
    return pair_and_col;
}
rlim_t mb_to_bytes(int mb){
    return (rlim_t) (mb) * 1000000;
}
void set_limits(int cpu_limit, int vm_limit_mb){
    struct rlimit cpu = {(rlim_t) cpu_limit, (rlim_t) cpu_limit};
    rlim_t as_limit = mb_to_bytes(vm_limit_mb);
    struct rlimit as = {as_limit, as_limit};
    setrlimit(RLIMIT_CPU, &cpu);
    setrlimit(RLIMIT_AS, &as);
}
int worker_function(struct matrix **a, struct matrix **b, int max_time, int mode, char **out_file, int total_pairs,
        int cpu_time_limit, int vm_limit) {
    set_limits(cpu_time_limit, vm_limit);
    time_t start_time = time(NULL);
    int task_nr = 0;
    while (1) {
        if ((time(NULL) - start_time) >= max_time) {
            printf("max_time reached\n");
            break;
        }
        int *pair_col = get_task(total_pairs);
        if (pair_col[1] == -1) {
            printf("all tasks executed\n");
            break;
        }
        if(mode == MODE_JOINT)
            calc_col_in_mx(a[pair_col[0]], b[pair_col[0]], pair_col[1], out_file[pair_col[0]]);
        else
            calc_separate_col(a[pair_col[0]], b[pair_col[0]], pair_col[1], pair_col[0]);
        task_nr++;
    }
    return task_nr;
}

int number_of_lines(FILE* file) {
    fseek(file, 0, SEEK_SET);
    char buffer[LINE_BUFF];
    int lines = 0;
    while (fgets(buffer, LINE_BUFF, file) != NULL) lines++;
    return lines;
}

int main(int argc, char* argv[]) {
    if (argc != 7) {
        printf("wrong argument number (expected 4, got %d)", argc-1);
        return 1;
    }
    //// read arguments:
    FILE* list = fopen(argv[1], "r");
    int workers_nr = atoi(argv[2]);
    int max_time = atoi(argv[3]);
    int mode = strcmp(argv[4], "joint") == 0 ? MODE_JOINT : MODE_DISJOINT;
    int pc_time_limit = atoi(argv[5]); // in s
    int vm_limit = atoi(argv[6]); // in mb

    char **c_files = calloc(100, sizeof(char*));
    int total_pairs = number_of_lines(list);
    struct matrix** as = calloc((size_t) total_pairs, sizeof(struct matrix));
    struct matrix** bs = calloc((size_t) total_pairs, sizeof(struct matrix));

    create_out_folder();

    char line[PATH_MAX * 3 + 3];
    int pair_idx = 0;
    fseek(list, 0, SEEK_SET);
    while (fgets(line, PATH_MAX*3 + 3, list) != NULL) {
        c_files[pair_idx] = calloc(PATH_MAX, sizeof(char));
        char* aF = strtok(line, " ");
        char* bF = strtok(NULL, " ");
        strcpy(c_files[pair_idx], strtok(NULL, " "));
        //// trimming \n from c filename
        size_t len = strlen(c_files[pair_idx]);
        c_files[pair_idx][len-1] = 0;
        as[pair_idx] = load_mx(aF);
        bs[pair_idx] = load_mx(bF);
        if(mode == MODE_JOINT) get_random_mx(as[pair_idx]->row_nr, bs[pair_idx]->col_nr, c_files[pair_idx]);

        char* task_filename = calloc(100, sizeof(char));
        sprintf(task_filename, "%s/tasks%03d", out_folder, pair_idx);
        FILE* tasks_file = fopen(task_filename, "w+");

        char* tasks = calloc((bs[pair_idx]->col_nr + 1), sizeof(char));
        sprintf(tasks, "%0*d", bs[pair_idx]->col_nr, 0);
        fwrite(tasks, 1, bs[pair_idx]->col_nr, tasks_file);
        free(tasks);
        free(task_filename);
        fclose(tasks_file);

        pair_idx++;
    }
    pid_t* workers = calloc((size_t) workers_nr, sizeof(int));
    for (int i = 0; i < workers_nr; i++) {
        pid_t worker = fork();
        if (worker == 0) return worker_function(as, bs, max_time, mode, c_files, total_pairs, pc_time_limit, vm_limit);
        else workers[i] = worker;
    }
    struct rusage start_time;
    struct rusage end_time;
    for (int i = 0; i < workers_nr; i++) {
        int res;
        getrusage(RUSAGE_CHILDREN, &start_time);
        waitpid(workers[i], &res, 0);
        getrusage(RUSAGE_CHILDREN, &end_time);
        printf("Process nr %d executed %d matrix multiplications\n", workers[i], WEXITSTATUS(res));
        print_stats(&start_time, &end_time);

    }
    free(workers);

    if(mode == MODE_DISJOINT){
        for(int i=0; i<pair_idx; i++){
            char *buffer = calloc(1000, sizeof(char));
            sprintf(buffer, "paste %s/part%02d* -d' '> %s", out_folder, i, c_files[i]);
            system(buffer);
        }
    }
    ///// free memory
    for(int i=0;i<total_pairs;i++){
        free_mx(as[i]);
        free_mx(bs[i]);
        free(c_files[i]);
    }
    free(as);
    free(bs);
    free(c_files);
    return 0;
}

void print_stats(struct rusage *start_t, struct rusage *end_t) {
    time_t user_sec = end_t->ru_utime.tv_sec - start_t->ru_utime.tv_sec;
    time_t user_micro = abs(end_t->ru_utime.tv_usec - start_t->ru_utime.tv_usec);

    time_t system_sec = end_t->ru_stime.tv_sec - start_t->ru_stime.tv_sec;
    time_t system_micro = abs(end_t->ru_stime.tv_usec - start_t->ru_stime.tv_usec);

    printf("user time: %ld.%06ld\n", user_sec, user_micro);
    printf("system time: %ld.%06ld\n\n", system_sec, system_micro);
}

void create_out_folder() {
    char cmd[500];
    snprintf(cmd, 500, "rm -rf %s", out_folder);
    system(cmd);
    snprintf(cmd, 500, "mkdir -p %s", out_folder);
    system(cmd);
}
