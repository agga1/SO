#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/limits.h>
#include <stdbool.h>
#include "matrix_manage.c"

bool check_multiply_correctness(char *a_filename, char *b_filename, char *c_filename){
    struct matrix *a = load_mx(a_filename);
    struct matrix *b = load_mx(b_filename);
    struct matrix *c = load_mx(c_filename);
    printf("%d\n", c->col_nr);
    struct matrix *correct_matrix = dot(a, b);
    if(correct_matrix->col_nr != c->col_nr || correct_matrix->row_nr != c->row_nr) return false;
    for( int i=0; i<correct_matrix->row_nr; i++){
        for(int j=0; j<correct_matrix->col_nr; j++){
            if(correct_matrix->mx[i][j] != c->mx[i][j]) return false;
        }
    }
    free_mx(a);
    free_mx(b);
    free_mx(c);
    free_mx(correct_matrix);
    return true;
}
char *create_name(char* folder, char name, int nr){
    char *res = calloc(100, sizeof(char));
    snprintf(res, 100, "%s/%c%d.txt", folder,name, nr );
    return res;
}
void create_mxs(int min, int max, int pairs_nr, char* to_folder) {
    char *cmd = calloc(500, sizeof(char));
    sprintf(cmd, "mkdir %s", to_folder);
    system(cmd);
    for (int i = 0; i < pairs_nr; i++) {
        int a_rows = get_random(min, max);
        int a_cols = get_random(min, max);
        int b_cols = get_random(min, max);
        char *a_name = create_name(to_folder, 'a', i);
        char *b_name = create_name(to_folder, 'b', i);
        char *c_name = create_name(to_folder, 'c', i);
        get_random_mx(a_rows, a_cols, a_name);
        get_random_mx(a_cols, b_cols, b_name);

        snprintf(cmd, 500, "echo \"%s %s %s\" >> lista", a_name, b_name, c_name);
        system(cmd);
    }
}
int main(int argc, char** argv){
    if (argc != 5) {
        printf("wrong number of arguments (expected 4, got %d)", argc-1);
        return 1;
    }
    if(strcmp(argv[1], "create") == 0){
        srand((unsigned int) time(NULL));
        int min = atoi(argv[2]);
        int max = atoi(argv[3]);
        int pairs_nr = atoi(argv[4]);
        create_mxs(min, max, pairs_nr, "matrixes");
    } else if(strcmp( argv[1], "check") == 0){
        bool ok = check_multiply_correctness(argv[2], argv[3], argv[4]);
        if(ok) printf("multiplication correct!\n");
        else printf("multiplication incorrect!\n");

    }else{
        fprintf(stderr, "no such command");
    }
    return 0;
}
