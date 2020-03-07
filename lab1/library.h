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
struct main_array* create_table(int size);
void compare_pairs(char *pairs, int nr_of_pairs);
void compare_pair( char *pair, char *out_filename);
void remove_block(int index, struct main_array* ma);
void remove_ed_op(int b_index, int ed_op_index, struct main_array* ma);
struct block* process_tmp_file(char *filename);
#endif