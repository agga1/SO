#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* my_dir_tmp = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/ex3/tmp_files/";
char* my_dir_txt = "/mnt/d/Agnieszka/Documents/Studia/4semestr/SO/lab1/ex3/txt_files/";

FILE* load_file(char* filename){
    if(filename == NULL) return NULL;
    char _path[500];
    snprintf(_path, sizeof(_path), "%s%s", my_dir_tmp, filename);
    FILE* filestream = fopen(_path,"r");
    if(filestream == NULL) {
        perror("file not found");
        exit(-1);
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
        perror("number of found and declared pairs do not match");
        exit(-1);
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
//// all-in-one
struct main_array * compare_pairs(char *pairs, int nr_of_pairs){
    char **pair_names = seq_to_pair_array(pairs, nr_of_pairs);
    compare_to_tmp_file(pair_names, nr_of_pairs);
    struct main_array* ma = main_array_new(nr_of_pairs);
    tmp_to_array(ma);
    return ma;
}
//// all-in-one with already initializaed array
void compare_pairs_to_array(char *pairs, struct main_array * ma){
    char **pair_names = seq_to_pair_array(pairs, ma->size);
    compare_to_tmp_file(pair_names, ma->size);
    tmp_to_array(ma);
}
void compare_pair_to_tmp(char *pair, char *out_filename) { // pair like "file1.txt:file2.txt"
    char *file_a = strtok(pair, ":");
    char *file_b = strtok(NULL, ":");
    char command[512];
    char out_path[500];
    snprintf(out_path, sizeof(out_path), "%s%s", my_dir_tmp, out_filename);
    snprintf(command, sizeof(command), "cd %s && diff  %s %s > %s", my_dir_txt, file_a, file_b, out_path);
    system(command);
}
struct block* process_tmp_file(char *filename){
    if(filename == NULL) return NULL;

    char _path[500];
    snprintf(_path, sizeof(_path), "%s%s", my_dir_tmp, filename);
    FILE *fptr = fopen(_path,"r");

    if( fptr == NULL ){
        perror("cannot open file");
        exit(-1);
    }

    int buf_size=255;
    char buffer[buf_size];
    int op_nr=0, size=1;
//// counts number of ed_ops
    while(fgets(buffer,buf_size,fptr)!=NULL){
        if(buffer[0]<='9' && buffer[0]>='0')         // 60=='<' 62=='>' 45=='-'
            op_nr++;
        size+=strlen(buffer);
    }
    struct block* b=block_new(op_nr);
    rewind(fptr);

    char *str=(char*)calloc((size_t) size, sizeof(char));
    int k=0;
    strcpy(str,"");
//// saves array of ed_ops
    while(fgets(buffer,buf_size,fptr)!=NULL){
        if(buffer[0]<='9' && buffer[0]>='0'){

            if(strcmp(str,"")!=0){
                b->a[k]= (char*) calloc (strlen(str)+1, sizeof(char));
                strcpy(b->a[k],str);
                k++;
                strcpy(str,"");
            }
            strcpy(str,buffer);
        }
        else{
            strcat(str,buffer);
        }
    }
    if(strcmp(str, "")!=0){
        b->a[k] = (char*) calloc (strlen(str)+1, sizeof(char));
        strcpy(b->a[k],str);               //adds last operations
    }

    fclose(fptr);
    free(str);
    return b;
}
//// save single block from tmp_{idx}.txt file
void save_block(int idx, struct main_array* ma){
    char filename[50];
    snprintf(filename, sizeof(filename), "tmp_%d.txt", idx);
    struct block* b = process_tmp_file(filename);  // populating each block w pointers to ed_ops
    ma->blocks[idx] = b;
}
int remove_block(int idx, struct main_array* ma){
    if(ma == NULL || ma->blocks[idx] == NULL || idx < 0 || idx >= ma->size ) return -1;
    free(ma->blocks[idx]);
    ma->blocks[idx]=NULL;

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
