#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const char* path = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/ex1/txt_files/";

struct main_array* create_table(int size) {
    return main_array_new(size);
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
char ** seq_to_pairs(char *pairs, int nr_of_pairs){ //// split string into array of pairs
    int pairs_nr=0;
    char **pair_names = calloc((size_t) nr_of_pairs, sizeof(char*));
    char * token = strtok(pairs, " ");
    while( token != NULL ) {
        char *pair = calloc(strlen(token), sizeof(char));
        strcpy(pair, token);
        pair_names[pairs_nr] = pair;
        pairs_nr +=1;
        token = strtok(NULL, " ");
    }
    if(nr_of_pairs != pairs_nr){
        printf("%d pairs found, %d declared", pairs_nr, nr_of_pairs);
        return NULL;
    }
    return pair_names;
}
void compare_to_tmp_file(char **pair_names, int pairs_nr){
    for(int i=0;i<pairs_nr;i++){
        char out_file[50];
        snprintf(out_file, sizeof(out_file), "tmp_%d.txt", i);
        compare_pair(pair_names[i], out_file); // comparing each pair of files
    }
}
struct main_array* tmp_to_array(int pairs_nr){
    struct main_array* ma = main_array_new(pairs_nr);
    for(int i=0;i<pairs_nr;i++){
        char filename[50];
        snprintf(filename, sizeof(filename), "tmp_%d.txt", i);
        struct block* b = process_tmp_file(filename); // populating each block w pointers to ed_ops
        printf("size of block no %d = %d",i, b->size);
        ma->blocks[i] = b;
    }
    return ma;
}
void compare_pairs(char *pairs, int nr_of_pairs){
    //// split string into array of pairs
    int pairs_nr=0;
    char *pair_names[nr_of_pairs];
    char * token = strtok(pairs, " ");
    while( token != NULL ) {
        char *pair = calloc(strlen(token), sizeof(char));
        strcpy(pair, token);
        pair_names[pairs_nr] = pair;
        pairs_nr +=1;
        token = strtok(NULL, " ");
    }
    if(nr_of_pairs != pairs_nr){
        printf("%d pairs found, %d declared", pairs_nr, nr_of_pairs);
        return;
    }
    //// creating tmp files for each pair
    for(int i=0;i<pairs_nr;i++){
        char out_file[50];
        snprintf(out_file, sizeof(out_file), "tmp_%d.txt", i);
        compare_pair(pair_names[i], out_file); // comparing each pair of files
    }

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
void compare_pair(char *pair, char *out_filename) { // pair like "file1.txt:file2.txt"
    char *file_a = strtok(pair, ":");
    char *file_b = strtok(NULL, ":");
    char command[512];
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
//                printf("b->a[%d]=%s", i, b->a[i]);
                i++;
                strcpy(ed_op, "");
            }
        }
        strcat(ed_op, line);
    }
    if(strcmp(ed_op, "") != 0) // last editing operation
    {
        b->a[i] = malloc(strlen(ed_op)* sizeof(char));
        strcpy(b->a[i], ed_op);
    }

    fclose(tmp_file);
    if (line)
        free(line);
    return b;
}

int remove_block(int index, struct main_array* ma){
    if(ma == NULL || ma->blocks[index] == NULL || index < 0 || index >= ma->size ) return -1;
    free(ma->blocks[index]);
    ma->blocks[index]=NULL;

    return 0;

}
int remove_ed_op(int b_index, int ed_op_index, struct main_array* ma){
    if(ma == NULL|| b_index < 0 || b_index >= ma->size  || ma->blocks[b_index] == NULL  || ed_op_index > ma->blocks[b_index]->size) return -1;
    free(ma->blocks[b_index]->a[ed_op_index]);
    ma->blocks[b_index]->a[ed_op_index]=NULL;
    return 0;
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
