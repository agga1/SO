#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/times.h>

const int N=3;
const char* commands[]= {"generate", "sort", "copy"};
bool in_commands(char* el){
    for(int i = 0; i < N; i++) {
        if(strcmp(commands[i], el) == 0)
            return true;
    }
    return false;
}
char* file_path(char *file_name);
int generate(char *file_path, int records_no, size_t size );
int sort_lib(char *file_path, int records_no, size_t size);
int sort_sys(char *file_path, int records_no, size_t size);
int copy_lib(char *from_path, char *to_path, int records_no, size_t bytes);
int copy_sys(char *from_path, char *to_path, int records_no, size_t bytes);

int main(int argc, char **argv) {
    if(argc<2){
        printf("no action specified");
        return 1;
    }
    int nr=1;
    if (strcmp(argv[nr], commands[0]) == 0) { // generate
        if(nr+3>=argc){
            printf("not enough arguments for generate, provide [filename], [record_nr], [record_bytes(size_t)]\n");
            return 2;
        }
        char *filename = file_path(argv[++nr]);
        int record_nr = atoi(argv[++nr]);
        size_t record_bytes= (size_t) atoi(argv[++nr]);
        generate(filename, record_nr, record_bytes);
//            printf("generated\n");
//        else
//            perror("files not generated");
    }
    else if (strcmp(argv[nr], commands[1]) == 0) { // sort
        printf("sorta");
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
        printf("cp");
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
    return 0;
}
int generate(char *file_path, int records_no, size_t size){
    FILE *my_file = fopen(file_path, "w+");

    char *buffer = malloc((size)* sizeof(char));
    for (int i = 0; i < records_no; i++) {
        for (int j = 0; j < size; j++) {
            buffer[j] = (char) ('a' + (rand() % 26));
        }
        buffer[size-1] = '\n';
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

void swap_lib(FILE* file, char* i_line, char* j_line, int i, int j, size_t length){
    fseek(file, i * length * sizeof(char), SEEK_SET);
    fwrite(j_line, sizeof(char), length, file);
    fseek(file, j * length * sizeof(char), SEEK_SET);
    fwrite(i_line, sizeof(char), length, file);
}

void sort_lib_body(FILE *file, size_t size, int start, int end){
    if(start > end) return;
    char *pivot = calloc(size, sizeof(char));
    fseek(file, end * size * sizeof(char), SEEK_SET);
    fread(pivot, sizeof(char), size, file);

    char *curr = calloc(size, sizeof(char));

    int i = start;

    for (int j = start; j < end; ++j) {
        fseek(file, j * size * sizeof(char), SEEK_SET);
        fread(curr, sizeof(char), size, file);

        if (strcmp(curr, pivot) < 0) {
            fseek(file, i * size * sizeof(char), SEEK_SET);
            fread(pivot, sizeof(char), size, file);
            swap_lib(file, pivot, curr, i, j, size);
            i++;
            fseek(file, end * size * sizeof(char), SEEK_SET);
            fread(pivot, sizeof(char), size, file);
        }
    }
    fseek(file, i * size * sizeof(char), SEEK_SET);
    fread(curr, sizeof(char), size, file);
    swap_lib(file, curr, pivot, i, end, size);
    free(pivot);
    free(curr);
    sort_lib_body(file, size, start, i - 1);
    sort_lib_body(file, size, i + 1, end);
}

int sort_lib(char *file_path, int lines, size_t size){
    FILE *handle = fopen(file_path, "r+");
    sort_lib_body(handle, size,  0, lines - 1);
    fclose(handle);
    return 0;
}
void swap_sys(int file, char* i_line, char* j_line, int i, int j, size_t length){
    lseek(file, i * length * sizeof(char), SEEK_SET);
    write(file, j_line, length);
    lseek(file, j * length * sizeof(char), SEEK_SET);
    write(file, i_line, length);
}

void sort_sys_body(int fd, size_t size, int start, int end){
    if(start > end) return; // wyjście rekurencji

    char *pivot = calloc(size, sizeof(char));
    lseek(fd, end * size * sizeof(char), SEEK_SET);
    read(fd, pivot, size);

    char *curr = calloc(size, sizeof(char));

    int i = start;

    for (int j = start; j < end; ++j) {
        lseek(fd, j * size * sizeof(char), SEEK_SET);
        read(fd, curr, size);

        if (strcmp(curr, pivot) < 0) {
            lseek(fd, i * size * sizeof(char), SEEK_SET);
            read(fd, pivot, size);
            swap_sys(fd, pivot, curr, i, j, size);
            i++;
            lseek(fd, end * size * sizeof(char), SEEK_SET);
            read(fd, pivot, size);
        }
    }
    lseek(fd, i * size * sizeof(char), SEEK_SET);
    read(fd, curr, size);
    swap_sys(fd, curr, pivot, i, end, size);
    free(pivot);
    free(curr);
    sort_sys_body(fd, size, start, i - 1);
    sort_sys_body(fd, size, i + 1, end);
}

int sort_sys(char *file_path, int lines, size_t size){
    int fd = open(file_path, O_RDWR);
    sort_sys_body(fd, size, 0, lines - 1);
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

