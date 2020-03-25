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
const char* out_folder = "tmp";
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
int* get_task(int total_pairs) {
    int *pair_and_col = calloc(2, sizeof(int));
    pair_and_col[0]=pair_and_col[1]=-1;

    for(int pair_idx=0; pair_idx < total_pairs; pair_idx++){
        char* filename = calloc(100, sizeof(char));
        sprintf(filename, "%s/tasks%03d",out_folder, pair_idx);
        FILE* tasksF = fopen(filename, "r+");
        // get file descr and lock file
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
            // update done tasks
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
int worker_function(char** a, char** b, int max_time, int mode, char **out_file, int total_pairs) {
    time_t start_time = time(NULL);
    int task_nr = 0;
    while (1) {
        if ((time(NULL) - start_time) >= max_time) {
            printf("max_time reached\n");
            break;
        }
        int *pair_col = get_task(total_pairs);
//        printf("solving pair nr %d, col nr %d\n", pair_col[0], pair_col[1]);
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
        sprintf(task_filename, "%s/tasks%03d", out_folder, pairs_nr);
        FILE* tasks_file = fopen(task_filename, "w+");

        char* tasks = calloc(b->col_nr + 1, sizeof(char));
        sprintf(tasks, "%0*d", b->col_nr, 0);
        fwrite(tasks, 1, b->col_nr, tasks_file);
//        printf("new tasks %s", tasks);
        free(tasks);
        free(task_filename);
        fclose(tasks_file);

        pairs_nr++;
    }
    int total_pairs = pairs_nr;

    pid_t* processes = calloc(workers_nr, sizeof(int));
    for (int i = 0; i < workers_nr; i++) {
        pid_t worker = fork();
        if (worker == 0) { // child process
            return worker_function(a_files, b_files, max_time, mode, c_files, total_pairs);
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
