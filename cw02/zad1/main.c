#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <sys/resource.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/times.h>

const int N=5;
const char* commands[]= {"generate", "sort", "copy", "write", "comment"};
bool in_commands(char* el){
    for(int i = 0; i < N; i++) {
        if(strcmp(commands[i], el) == 0)
            return true;
    }
    return false;
}
char *dir() {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    return cwd;
}
void append_file(FILE* result_file, char* data){
    fprintf(result_file, "%s\n", data);
}
void write_time(FILE *result_file, double r, double u, double s){
    fprintf(result_file,"   Real   |   User   |   System\n");
    fprintf(result_file, " %f  %f  %f\n\n", r, u, s);
}
double calc_time(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}
char* file_path(char *file_name);
int generate(char *file_path, int records_no, size_t size );
int sort_lib(char *file_path, int record_no, size_t size);
int sort_sys(char *file_path, int record_no, size_t size);
int copy_lib(char *from_path, char *to_path, int records_no, size_t bytes);
int copy_sys(char *from_path, char *to_path, int records_no, size_t bytes);

int main(int argc, char **argv) {
    if(argc<2){
        printf("no action specified");
        return 1;
    }

    //// time measurement
    struct tms **tms_time = malloc(6 * sizeof(struct tms *));
    clock_t real_time[2];
    for (int i = 0; i < 2; i++) tms_time[i] = (struct tms *) malloc(sizeof(struct tms *));
    real_time[0] = times(tms_time[0]);

    /// output file - specify option!
    const char* cwd = dir();
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s/out/%s", cwd, "wyniki.txt");
    FILE *result_file;
    int nr=1;
    if(strcmp(argv[nr], commands[3])==0) {result_file = fopen(path, "w"); nr++;}
    else result_file = fopen(path, "a");
    if(strcmp(argv[nr], commands[4])==0)  {nr++; append_file(result_file, argv[nr]); nr++;}

    if (strcmp(argv[nr], commands[0]) == 0) { // generate
        if(nr+3>=argc){
            printf("not enough arguments for generate, provide [filename], [record_nr], [record_bytes(size_t)]\n");
            return 2;
        }
        char *filename = file_path(argv[++nr]);
        int record_nr = atoi(argv[++nr]);
        size_t record_bytes= (size_t) atoi(argv[++nr]);
        generate(filename, record_nr, record_bytes);
    }
    else if (strcmp(argv[nr], commands[1]) == 0) { // sort
        if(nr+4>=argc){
            printf("not enough arguments for sort, provide [filename], [record_nr], [record_size_t], [sys|lib]\n");
            return 2;
        }
        char *filename = file_path(argv[++nr]);
        int record_nr = atoi(argv[++nr]);
        size_t record_bytes= (size_t) atoi(argv[++nr]);
        if(strcmp(argv[++nr], "sys") == 0)
            sort_sys(filename, record_nr, record_bytes);
        else if(strcmp(argv[nr], "lib") == 0)
            sort_lib(filename, record_nr, record_bytes);
        else{
            printf("argument sys/lib not found");
            return 3;
        }
    }
    else if (strcmp(argv[nr], commands[2]) == 0) { // copy
        if(nr+5>=argc){
            printf("not enough arguments for copy, provide [filename1], [filename2], [record_nr], [record_size_t], [sys|lib]\n");
            return 2;
        }
        char *file_path1 = file_path(argv[nr+1]);
        char *file_path2 = file_path(argv[nr+2]);
        int record_nr = atoi(argv[nr+3]);
        size_t record_bytes= (size_t) atoi(argv[nr+4]);
        if(strcmp(argv[nr+5], "sys") == 0)
            copy_sys(file_path1,file_path2, record_nr, record_bytes);
        else if(strcmp(argv[nr+5], "lib") == 0)
            copy_lib(file_path1, file_path2, record_nr, record_bytes);
        else{
            printf("argument sys/lib not found");
            return 4;
        }
    }
    else{
        printf("no such command: %s", argv[nr]);
    }

    real_time[1] = times(tms_time[1]);
    double r= calc_time(real_time[0], real_time[1]);
    double u= calc_time(tms_time[0]->tms_utime, tms_time[1]->tms_utime);
    double s= calc_time(tms_time[0]->tms_stime, tms_time[1]->tms_stime);
    printf("   Real   |   User   |   System\n");
    printf("%lf   ", r);
    printf("%lf   ", u);
    printf("%lf ", s);
    printf("\n");


    write_time(result_file, r, u, s);
    fclose(result_file);
    return 0;
}
int generate(char *file_path, int records_no, size_t size){
    FILE *my_file = fopen(file_path, "w+");

    char *buffer = malloc((size)* sizeof(char));
    for (int i = 0; i < records_no; i++) {
        for (int j = 0; j < size; j++) {
            buffer[j] = (char) ('a' + (rand() % 26));
        }
//        buffer[size-1] = '\n';
        fwrite(buffer, sizeof(char), (size_t) size, my_file);
    }
    free(buffer);
    fclose(my_file);
    return 0;
}

char* file_path(char *file_name){
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    char *path = calloc(256, sizeof(char));
    snprintf(path, 256, "%s/out/%s", cwd, file_name);
    free(cwd);
    return path;
}

void lib_swap(FILE *file, char *line1, char *line2, int ind1, int ind2, size_t size){
    fseek(file, ind1 * size * sizeof(char), SEEK_SET);
    fwrite(line2, sizeof(char), size, file);
    fseek(file, ind2 * size * sizeof(char), SEEK_SET);
    fwrite(line1, sizeof(char), size, file);
}
int lib_partition(FILE* file, size_t size, int low, int high){
    char *pivot = calloc(size, sizeof(char));
    fseek(file, high * size * sizeof(char), SEEK_SET);
    fread(pivot, sizeof(char), size, file);

    char *curr = calloc(size, sizeof(char));

    int i = low;

    for (int j = low; j < high; ++j) {
        fseek(file, j * size * sizeof(char), SEEK_SET);
        fread(curr, sizeof(char), size, file);

        if (strcmp(curr, pivot) < 0) {
            fseek(file, i * size * sizeof(char), SEEK_SET);
            fread(pivot, sizeof(char), size, file);
            lib_swap(file, pivot, curr, i, j, size);
            i++;
            fseek(file, high * size * sizeof(char), SEEK_SET);
            fread(pivot, sizeof(char), size, file);
        }
    }
    fseek(file, i * size * sizeof(char), SEEK_SET);
    fread(curr, sizeof(char), size, file);
    lib_swap(file, curr, pivot, i, high, size);
    free(pivot);
    free(curr);
    return i;
}
void lib_qsort(FILE *file, size_t size, int low, int high){
    if (low < high) {
        int pivot = lib_partition(file, size, low, high);
        lib_qsort(file, size, low, pivot - 1);
        lib_qsort(file, size, pivot + 1, high);
    }
}

int sort_lib(char *file_path, int record_no, size_t size){
    FILE *handle = fopen(file_path, "r+");
    lib_qsort(handle, size, 0, record_no - 1);
    fclose(handle);
    return 0;
}
void sys_swap(int fd, char *line1, char *line2, int ind1, int ind2, size_t size){
    lseek(fd, ind1 * size * sizeof(char), SEEK_SET);
    write(fd, line2, size);
    lseek(fd, ind2 * size * sizeof(char), SEEK_SET);
    write(fd, line1, size);
}
int sys_partition(int fd, size_t size, int low, int high){
    char *pivot = calloc(size, sizeof(char));
    lseek(fd, high * size * sizeof(char), SEEK_SET);
    read(fd, pivot, size);

    char *curr = calloc(size, sizeof(char));

    int i = low;
    for (int j = low; j < high; j++) {
        lseek(fd, j * size * sizeof(char), SEEK_SET);
        read(fd, curr, size);

        if (strcmp(curr, pivot) < 0) {
            lseek(fd, i * size * sizeof(char), SEEK_SET);
            read(fd, pivot, size);
            sys_swap(fd, pivot, curr, i, j, size);
            i++;
            lseek(fd, high * size * sizeof(char), SEEK_SET);
            read(fd, pivot, size);
        }
    }
    lseek(fd, i * size * sizeof(char), SEEK_SET);
    read(fd, curr, size);
    sys_swap(fd, curr, pivot, i, high, size);
    free(pivot);
    free(curr);
    return i;
}
void sys_qsort(int fd, size_t size, int start, int end){
    if (start < end) {
        int pi = sys_partition(fd, size, start, end);
        sys_qsort(fd, size, start, pi - 1);
        sys_qsort(fd, size, pi + 1, end);
    }
}

int sort_sys(char *file_path, int record_no, size_t size){
    int fd = open(file_path, O_RDWR);
    sys_qsort(fd, size, 0, record_no - 1);
    close(fd);
    return 0;
}
/*
 * copies @records_no records of size @bytes using c library functions
 */
int copy_lib(char *from_path, char *to_path, int records_no, size_t bytes) {
    FILE *dest = fopen(to_path, "w");
    FILE *src = fopen(from_path, "r");
    char *buf = calloc(bytes, sizeof(char));
    for (int i = 0; i < records_no; ++i) {
        if(fread(buf, sizeof(char), bytes, src));
        fwrite(buf, sizeof(char), bytes, dest);
    }
    fclose(src);
    fclose(dest);
    free(buf);
    return 0;
}
/*
 * copies @records_no records of size @bytes using system functions
 */
int copy_sys(char *from_path, char *to_path, int records_no, size_t bytes){
    int src = open(from_path, O_RDONLY);
    if(src<0){
        printf("file not found");
        return -1;
    }
    int dest = open(to_path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    char *buf = calloc(bytes, sizeof(char));
    for (int i = 0; i < records_no; ++i) {
        read(src, buf, bytes);
        write(dest, buf, bytes);
    }
    close(dest);
    close(src);
    free(buf);
    return 0;
}

