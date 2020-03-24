#define _XOPEN_SOURCE 500
#define MODE_JOINT 1
#define MODE_DISJOINT 2
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "matrix_manage.c"
#include <time.h>
#include <unistd.h>
int pair_number = 0;

struct Task{
    int pair_index;
    int column_index;
};

struct Task get_task() {

    struct Task task;
    task.column_index = -1;
    task.pair_index = -1;
    for(int i=0; i< pair_number; i++){

        char* task_filename = calloc(100, sizeof(char));
        sprintf(task_filename, "tmp/tasks%d", i);
        FILE* tasks_file = fopen(task_filename, "r+");
        int fd = fileno(tasks_file);
        flock(fd, LOCK_EX);


        char* tasks = calloc(1000, sizeof(char));
        fseek(tasks_file, 0, SEEK_SET);
        fread(tasks, 1, 1000, tasks_file);

        char* task_first_zero = strchr(tasks, '0');
        int task_index = task_first_zero != NULL ? task_first_zero - tasks : -1;

        if (task_index >= 0) {
            char* end_of_line = strchr(tasks, '\0');
            int size = end_of_line - tasks;

            char* tasks_with_good_size = calloc(size +1, sizeof(char));
            for(int j=0; j<size; j++){
                tasks_with_good_size[j] = tasks[j];
            }
            tasks_with_good_size[task_index] = '1';
            fseek(tasks_file, 0, SEEK_SET);
            fwrite(tasks_with_good_size, 1, size, tasks_file);
            task.pair_index = i;
            task.column_index = task_index;
            flock(fd, LOCK_UN);
            fclose(tasks_file);
            break;
        }

        flock(fd, LOCK_UN);
        fclose(tasks_file);

    }

    return task;
}

void multiply_column(char* a_filename, char* b_filename, int col_index, int pair_index) {
    struct matrix *a = load_mx(a_filename);
    struct matrix *b = load_mx(b_filename);
    char* filename = calloc(20, sizeof(char));
    sprintf(filename, "tmp/part%d%04d", pair_index, col_index);
    FILE* part_file = fopen(filename, "w+");

    for (int y = 0; y < a->row_nr; y++) {
        int result = 0;

        for (int x = 0; x < a->col_nr; x++) {
            result += a->mx[y][x] * b->mx[x][col_index];
        }
        if(y== a->row_nr -1 ) fprintf(part_file, "%d ", result);
        else fprintf(part_file, "%d \n", result);
    }
    fclose(part_file);
}

void multiply_column_to_one_file(char* a_filename, char* b_filename, int col_index, char *result_file) {
    struct matrix *a = load_mx(a_filename);
    struct matrix *b = load_mx(b_filename);
    FILE* file = fopen(result_file, "r+");
    int fd = fileno(file);
    flock(fd, LOCK_EX);
    struct matrix *c = load_mx(result_file);

    for (int y = 0; y < a->row_nr; y++) {
        int result = 0;
        for (int x = 0; x < a->col_nr; x++) {
            result += a->mx[y][x] * b->mx[x][col_index];
        }
        c->mx[y][col_index] = result;
    }
    write_mx_to_file(file, c);
    flock(fd, LOCK_UN);
    fclose(file);
}

int worker_function(char** a, char** b, int timeout, int mode, char **result_file) {
    time_t start_time = time(NULL);
    int multiplies_count = 0;

    while (1) {
        if ((time(NULL) - start_time) >= timeout) {
            puts("timeout");
            break;
        }
        struct Task task = get_task();
        if (task.column_index == -1) {
            break;
        }
        if(mode == MODE_JOINT) {
            multiply_column_to_one_file(a[task.pair_index], b[task.pair_index], task.column_index,
                                        result_file[task.pair_index]);
        }
        else multiply_column(a[task.pair_index], b[task.pair_index], task.column_index, task.pair_index);

        multiplies_count++;
    }

    return multiplies_count;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("wrong argument number (expected 4, got %d)", argc-1);
        return 1;
    }

    FILE* list = fopen(argv[1], "r");
    int workers_nr = atoi(argv[2]);
    int max_time = atoi(argv[3]);
    int mode = strcmp(argv[4], "joint") == 0 ? MODE_JOINT : MODE_DISJOINT;

    char **a_files = calloc(100, sizeof(char*));
    char **b_files = calloc(100, sizeof(char*));
    char **c_files = calloc(100, sizeof(char*));

    system("rm -rf tmp");
    system("mkdir -p tmp");


    char line[PATH_MAX * 3 + 3];
    int pairs_nr = 0;
    while (fgets(line, PATH_MAX*3 + 3, list) != NULL) {

        a_files[pairs_nr] = calloc(PATH_MAX, sizeof(char));
        b_files[pairs_nr] = calloc(PATH_MAX, sizeof(char));
        c_files[pairs_nr] = calloc(PATH_MAX, sizeof(char));

        strcpy(a_files[pairs_nr], strtok(line, " "));
        strcpy(b_files[pairs_nr], strtok(NULL, " "));
        strcpy(c_files[pairs_nr], strtok(NULL, " "));

        struct matrix *a = load_mx(a_files[pairs_nr]);
        struct matrix *b = load_mx(b_files[pairs_nr]);
        if(mode == MODE_JOINT) generate_matrix(a->row_nr, b->col_nr, c_files[pairs_nr]);

        char* task_filename = calloc(100, sizeof(char));
        sprintf(task_filename, "tmp/tasks%d", pairs_nr);
        FILE* tasks_file = fopen(task_filename, "w+");

        char* tasks = calloc(b->col_nr + 1, sizeof(char));
        sprintf(tasks, "%0*d", b->col_nr, 0);
        fwrite(tasks, 1, b->col_nr, tasks_file);
        free(tasks);
        free(task_filename);
        fclose(tasks_file);

        pairs_nr++;
    }
    pair_number = pairs_nr;

    pid_t* processes = calloc(workers_nr, sizeof(int));
    for (int i = 0; i < workers_nr; i++) {
        pid_t worker = fork();
        if (worker == 0) {
            return worker_function(a_files, b_files, max_time, mode, c_files);
        } else {
            processes[i] = worker;
        }
    }

    for (int i = 0; i < workers_nr; i++) {
        int status;
        waitpid(processes[i], &status, 0);
        printf("Process nr %d executed %d matrix multiplications\n", processes[i],
               WEXITSTATUS(status));
    }
    free(processes);

    if(mode == MODE_DISJOINT){
        for(int i=0; i<pairs_nr; i++){
            char *buffer = calloc(1000, sizeof(char));
            sprintf(buffer, "paste tmp/part%d* > %s", i, c_files[i]);
            system(buffer);
        }

    }
    return 0;
}
