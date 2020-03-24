#define _XOPEN_SOURCE 500
#define MODE_JOINT 1
#define MODE_DISJOINT 2
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
int total_pairs = 0;
const char* out_folder = "tmp";
struct Task{
    int idx;
    int col;
};
int task_for_pair(int pair_idx, int total_tasks){
    char buffer[PATH_MAX + 1];
    sprintf(buffer, ".tmp/tasks%03d", pair_idx);
    FILE* tasks_file = fopen(buffer, "r+");
    int fd = fileno(tasks_file);
    flock(fd, LOCK_EX);

    char* tasks = calloc(total_tasks + 1, sizeof(char));
    fseek(tasks_file, 0, SEEK_SET);
    fread(tasks, 1, total_tasks, tasks_file);

    char* task_ptr_offset = strchr(tasks, '0');
    int task_index = task_ptr_offset != NULL ? task_ptr_offset - tasks : -1;

    if (task_index >= 0) {
        tasks[task_index] = '1';
        fseek(tasks_file, 0, SEEK_SET);
        fwrite(tasks, 1, total_tasks, tasks_file);
        fflush(tasks_file);
    }

    free(tasks);

    flock(fd, LOCK_UN);
    fclose(tasks_file);

    return task_index;
}
struct Task get_task() {

    struct Task task;
    task.col = -1;
    task.idx = -1;
    for(int i=0; i< total_pairs; i++){

        char* task_filename = calloc(100, sizeof(char));
        sprintf(task_filename, "%s/tasks%d",out_folder, i);
        FILE* tasks_file = fopen(task_filename, "r+");
        int fd = fileno(tasks_file);
        flock(fd, LOCK_EX);
        char* tasks = calloc(1000, sizeof(char));
        fseek(tasks_file, 0, SEEK_SET);
        fread(tasks, 1, 1000, tasks_file);

//        char* task_ptr_offset = strchr(tasks, '0');
//        int task_index = task_ptr_offset != NULL ? task_ptr_offset - tasks : -1;
//
//        if (task_index >= 0) {
//            tasks[task_index] = '1';
//            fseek(tasks_file, 0, SEEK_SET);
//            fwrite(tasks, 1, tasks_count, tasks_file);
//            fflush(tasks_file);
//        }
//
//        free(tasks);

        char* task_first_zero = strchr(tasks, '0');
        int task_index = task_first_zero != NULL ? task_first_zero - tasks : -1;

        if (task_index >= 0) {
            int size = (int) (strchr(tasks, '\0') - tasks);

            char* tasks_with_good_size = calloc(size +1, sizeof(char));
            for(int j=0; j<size; j++){
                tasks_with_good_size[j] = tasks[j];
            }
            tasks_with_good_size[task_index] = '1';
            fseek(tasks_file, 0, SEEK_SET);
            fwrite(tasks_with_good_size, 1, size, tasks_file);
            task.idx = i;
            task.col = task_index;
            flock(fd, LOCK_UN);
            fclose(tasks_file);
            break;
        }

        flock(fd, LOCK_UN);
        fclose(tasks_file);

    }
    return task;
}
// calculating column nr @col of output matrix to separate file
void calc_separate_col(char *a_filename, char *b_filename, int col, int pair_index) {
    struct matrix *a = load_mx(a_filename);
    struct matrix *b = load_mx(b_filename);
    char* filename = calloc(20, sizeof(char));
    sprintf(filename, "%s/part%d%04d", out_folder, pair_index, col);
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
// calculating column nr @col in output matrix
void calc_col_in_mx(char *a_file, char *b_file, int col, char *c_file) {
    struct matrix *a = load_mx(a_file);
    struct matrix *b = load_mx(b_file);
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
int worker_function(char** a, char** b, int max_time, int mode, char **out_file) {
    time_t start_time = time(NULL);
    int task_nr = 0;
    int pair_idx =0;
//    while (pair_idx<total_pairs) {
//        if ((time(NULL) - start_time) >= max_time) {
//            printf("max_time reached\n");
//            break;
//        }
//        int col = task_for_pair(pair_idx, b[pair_idx]->col);
//        if(mode == MODE_JOINT)
//            calc_col_in_mx(a[task.idx], b[task.idx], task.col, out_file[task.idx]);
//        else
//            calc_separate_col(a[task.idx], b[task.idx], task.col, task.idx);
//        task_nr++;
//    }
    while (1) {
        if ((time(NULL) - start_time) >= max_time) {
            printf("max_time reached\n");
            break;
        }
        struct Task task = get_task();
        if (task.col == -1) {
            printf("all tasks executed\n");
            break;
        }
        if(mode == MODE_JOINT)
            calc_col_in_mx(a[task.idx], b[task.idx], task.col, out_file[task.idx]);
        else
            calc_separate_col(a[task.idx], b[task.idx], task.col, task.idx);
        task_nr++;
    }
    return task_nr;
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

    char cmd[500];
    snprintf(cmd, 500, "rm -rf %s", out_folder);
    system(cmd);
    snprintf(cmd, 500, "mkdir -p %s", out_folder);
    system(cmd);

    char line[PATH_MAX * 3 + 3];
    int pairs_nr = 0;
    while (fgets(line, PATH_MAX*3 + 3, list) != NULL) {
        a_files[pairs_nr] = calloc(PATH_MAX, sizeof(char));
        b_files[pairs_nr] = calloc(PATH_MAX, sizeof(char));
        c_files[pairs_nr] = calloc(PATH_MAX, sizeof(char));

        strcpy(a_files[pairs_nr], strtok(line, " "));
        strcpy(b_files[pairs_nr], strtok(NULL, " "));
        strcpy(c_files[pairs_nr], strtok(NULL, " "));
        // trimming \n from c filename
        size_t len = strlen(c_files[pairs_nr]);
        c_files[pairs_nr][len-1] = 0;
        struct matrix *a = load_mx(a_files[pairs_nr]);
        struct matrix *b = load_mx(b_files[pairs_nr]);
        if(mode == MODE_JOINT) get_random_mx(a->row_nr, b->col_nr, c_files[pairs_nr]);

        char* task_filename = calloc(100, sizeof(char));
        sprintf(task_filename, "%s/tasks%d", out_folder, pairs_nr);
        FILE* tasks_file = fopen(task_filename, "w+");

        char* tasks = calloc(b->col_nr + 1, sizeof(char));
        sprintf(tasks, "%0*d", b->col_nr, 0);
        fwrite(tasks, 1, b->col_nr, tasks_file);
        free(tasks);
        free(task_filename);
        fclose(tasks_file);

        pairs_nr++;
    }
    total_pairs = pairs_nr;

    pid_t* processes = calloc(workers_nr, sizeof(int));
    for (int i = 0; i < workers_nr; i++) {
        pid_t worker = fork();
        if (worker == 0) { // child process
            return worker_function(a_files, b_files, max_time, mode, c_files);
        } else {
            processes[i] = worker;
        }
    }

    for (int i = 0; i < workers_nr; i++) {
        int res;
        waitpid(processes[i], &res, 0);
        printf("Process nr %d executed %d matrix multiplications\n", processes[i],
               WEXITSTATUS(res));
    }
    free(processes);

    // join result
    if(mode == MODE_DISJOINT){
        for(int i=0; i<pairs_nr; i++){
            char *buffer = calloc(1000, sizeof(char));
            sprintf(buffer, "paste %s/part%d* -d' '> %s", out_folder, i, c_files[i]);
            char name[400];
            snprintf(name, 400, "%s/part%d*", out_folder, i);
//            execlp("paste", "paste", name, ">", c_files[i], NULL);
            system(buffer);
        }

    }
    return 0;
}
