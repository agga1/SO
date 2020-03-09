#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>

struct block{
    int size; // how many editing operations there are
    char **a; // array of editing op.
};
struct block* block_new( int size);
struct main_array{
    int size; // how many blocks there are
    struct block** blocks; // array of blocks for each pair of files
};
struct main_array* main_array_new(int size);

void compare_pairs_to_array(char *pairs, struct main_array * ma);
struct main_array * compare_pairs(char *pairs, int nr_of_pairs); // same but initializes array as well

char ** seq_to_pair_array(char *pairs, int nr_of_pairs);
void compare_to_tmp_file(char **pair_names, int pairs_nr);
void compare_pair_to_tmp(char *pair, char *out_filename);
void tmp_to_array(struct main_array* ma);
void save_block(int idx, struct main_array* ma);

int remove_block(int index, struct main_array* ma);
int remove_ed_op(int b_index, int ed_op_index, struct main_array* ma);
struct block* process_tmp_file(char *filename);
#endif