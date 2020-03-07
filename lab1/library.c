#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char* path = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/";
struct block ** create_table(int size) {
    struct block** a =(struct block**)malloc(size*sizeof(struct block*));
    return a;
}

FILE* load_file(char* filename){
    if(filename == NULL) return NULL;
    char _path[500];
    snprintf(_path, sizeof(_path), "%s%s", path, filename);
    FILE* filestream = fopen(_path,"r");
    if(filestream == NULL) {
        printf("file %s not found", _path);
        return NULL;
    }
    return filestream;
}

void compare_pairs(char *pairs, int nr_of_pairs){
    //// creating tmp files with 'diff' command output
//    printf("%s", pairs);
    int pairs_nr=0;
    char *pair_names[nr_of_pairs];
    char * token = strtok(pairs, " ");
    while( token != NULL ) {
        char *pair = calloc(strlen(token), sizeof(char));
        strcpy(pair, token);
        char out_file[50];
        snprintf(out_file, sizeof(out_file), "tmp_%d.txt", pairs_nr);
        pairs_nr +=1;
        compare_pair(pair, out_file); // comparing each pair of files
        token = strtok(NULL, " ");
    }
    printf("%d pairs found", pairs_nr);

    //// populating main array with pointers to blocks
    struct main_array* ma = main_array_new(pairs_nr);
    for(int i=0;i<pairs_nr;i++){
        char filename[50];
        snprintf(filename, sizeof(filename), "tmp_%d.txt", i);
        struct block* b = process_tmp_file(filename); // populating each block w pointers to ed_ops
        printf("size of block no %d = %d",i, b->size);
        ma->blocks[i] = b;
    }
}
void compare_pair(char *pair, char *out_filename) {
    char * token = strtok(pair, ":");
    char * file_a = token;
    token = strtok(NULL, ":");
    char *file_b= token;
    char command[256];
    snprintf(command, sizeof(command), "cd %s && diff  %s %s > %s", path, file_a, file_b, out_filename);
    system(command);
}

struct block* process_tmp_file(char *filename){ // populates block with array of pointers to diff edit. op
    printf("\nprocessing %s... \n", filename);
    char * line = NULL;
    size_t len = 0;

    FILE* tmp_file = load_file(filename);
    if (tmp_file == NULL)
        exit(EXIT_FAILURE);

    int nr_ed_op = 0;
    while ((getline(&line, &len, tmp_file)) != -1) {
        if(line[0] <= '9' && line[0]>='0')
            nr_ed_op ++;
    }
    fclose(tmp_file);
    struct block* b=block_new(nr_ed_op);

    tmp_file = load_file(filename);
    char ed_op[1024];
    strcpy(ed_op, "");
    int i=0;
    while ((getline(&line, &len, tmp_file)) != -1) {
        if(line[0] <= '9' && line[0]>='0'){
            if(strcmp(ed_op, "") != 0) // not equal
            {
                b->a[i] = malloc(strlen(ed_op)* sizeof(char));
                strcpy(b->a[i], ed_op);
                i++;
                strcpy(ed_op, "");
            }
            strcat(ed_op, line);
        }
    }
    if(strcmp(ed_op, "") != 0) // last editing operation
    {
        b->a[i] = malloc(strlen(ed_op)* sizeof(char));
        strcpy(b->a[i], ed_op);
        strcpy(ed_op, "");
    }

    fclose(tmp_file);
    if (line)
        free(line);
    return b;
}
struct main_array* main_array_new(int size) {
    struct main_array* ma = malloc(sizeof(struct main_array));
    ma->size = size;
    ma->blocks = malloc(size*sizeof(struct block*));
    return ma;
}
struct block* block_new(int size) {
    struct block* b = malloc(sizeof(struct block));
    b->a = malloc(size* sizeof(char*));
    b->size = size;
    return b;
}
