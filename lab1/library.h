#ifndef LAB1_LIBRARY_H
#define LAB1_LIBRARY_H

#include <stdlib.h>
#include <stdio.h>

struct block{
    int size; // how many editing operations there are
    char **a;
};
struct block* block_new( int size);
struct main_array{
    int size;
    struct block** blocks;
};
struct main_array* main_array_new(int size);
struct block ** create_table(int size);
void compare_pairs(char *pairs, int nr_of_pairs);
void compare_pair( char *pair, char *out_filename);
struct block* process_tmp_file(char *filename);
#endif