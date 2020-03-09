#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* variab = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/ex2/txt_files/";

FILE* load_file(char* filename){
    if(filename == NULL) return NULL;
    char _path[500];
    snprintf(_path, sizeof(_path), "%s%s", variab, filename);
    FILE* filestream = fopen(_path,"r");
    if(filestream == NULL) {
        printf("file %s not found", _path);
        return NULL;
    }
    return filestream;
}

//// split string into array of pairs
char ** seq_to_pair_array(char *pairs, int nr_of_pairs){
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
    if(nr_of_pairs != pairs_nr){ // incorrect input msg
        printf("%d pairs found, %d declared", pairs_nr, nr_of_pairs);
        return NULL;
    }
    return pair_names;
}

//// compare each pair from pairs (pair like"a.txt:b.txt")
void compare_to_tmp_file(char **pair_names, int pairs_nr){
    for(int i=0;i<pairs_nr;i++){
        char out_file[50];
        snprintf(out_file, sizeof(out_file), "tmp_%d.txt", i);
        compare_pair_to_tmp(pair_names[i], out_file); // comparing each pair of files
    }
}

//// process tmp files (named "tmp_%d.txt", i<-0 until pairs_nr) and create array of blocks
void tmp_to_array(struct main_array* ma){
    for(int i=0;i<ma->size;i++){
        save_block(i, ma);
    }
}
struct main_array * compare_pairs(char *pairs, int nr_of_pairs){
    char **pair_names = seq_to_pair_array(pairs, nr_of_pairs);
    compare_to_tmp_file(pair_names, nr_of_pairs);
    struct main_array* ma = main_array_new(nr_of_pairs);
    tmp_to_array(ma);
    return ma;
}
void compare_pair_to_tmp(char *pair, char *out_filename) { // pair like "file1.txt:file2.txt"
    char *file_a = strtok(pair, ":");
    char *file_b = strtok(NULL, ":");
    char command[512];
    snprintf(command, sizeof(command), "cd %s && diff  %s %s > %s", variab, file_a, file_b, out_filename);
    printf("%s", command);
    system(command);
}
void compare_pairs_to_array(char *pairs, struct main_array * ma){
    char **pair_names = seq_to_pair_array(pairs, ma->size);
    compare_to_tmp_file(pair_names, ma->size);
    tmp_to_array(ma);
}

struct block* process_tmp_file(char *filename){ // populates block with array of pointers to diff edit. op
    printf("\nprocessing %s... \n", filename);
    char * line = NULL;
    size_t len = 0;

    FILE* tmp_file = load_file(filename);
    if (tmp_file == NULL)
        exit(EXIT_FAILURE);

    int nr_ed_op = 0;
    while ((getline(&line,(size_t*) &len, tmp_file)) != -1) {
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

//// save single block from tmp_{idx}.txt file
void save_block(int idx, struct main_array* ma){
    char filename[50];
    snprintf(filename, sizeof(filename), "tmp_%d.txt", idx);
    struct block* b = process_tmp_file(filename); // populating each block w pointers to ed_ops
    ma->blocks[idx] = b;
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
