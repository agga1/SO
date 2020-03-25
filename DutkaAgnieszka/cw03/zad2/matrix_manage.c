#define _XOPEN_SOURCE 500
#define LINE_BUFF 4096
#include <linux/limits.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>

struct matrix {
    int** mx;
    int row_nr;
    int col_nr;
};
struct matrix *new_matrix(int rows, int cols){
    struct matrix *m = malloc(sizeof(struct matrix));
    m->row_nr = rows;
    m->col_nr = cols;
    m->mx = calloc((size_t) rows, sizeof(int*));
    for (int y = 0; y < rows; y++) m->mx[y] = calloc((size_t) cols, sizeof(int));;
    return m;
}
int get_col_nr(char *row) { // count nr of columns based on one row
    int cols = 0;
    char* number = strtok(row, " ");
    while (number != NULL) {
        if(strcmp(number, "\n") != 0) cols++;
        number = strtok(NULL, " ");
    }
    return cols;
}
// load matrix from file
struct matrix *load_mx(char *path) {
    FILE *file = fopen(path, "r");
    if(file == NULL) perror("error loading matrix");
    printf("for path %s\n",  path);
    char line[LINE_BUFF];
    // get row and col nr
    int rows=0, cols=0;
    if(fgets(line, LINE_BUFF, file) != NULL){
        cols = get_col_nr(line);
        printf("check %d for path %s\n", cols, path);
        rows++;
        while (fgets(line, LINE_BUFF, file) != NULL)
            rows ++;
    }

    fseek(file, 0, SEEK_SET); // rewind to the beginning
    // read matrix
    struct matrix* m = new_matrix(rows, cols);
    int c, r = 0;
    while (fgets(line, LINE_BUFF, file) != NULL) {
        c = 0;
        char* val = strtok(line, " ");
        while (val != NULL) {
            m->mx[r][c++] = atoi(val);
            val = strtok(NULL, " ");
        }
        r++;
    }
    fclose(file);
    return m;
}
void free_mx(struct matrix *m) {
    for (int y = 0; y < m->row_nr; y++) free(m->mx[y]);
    free(m->mx);
}
struct matrix* dot(struct matrix *a, struct matrix *b){
    struct matrix *m = new_matrix(a->row_nr, b->col_nr);
    for (int i = 0; i < a->row_nr; i++) {
        for (int j = 0; j < b->col_nr; j++) {
            int result = 0;
            for(int k=0; k< a->col_nr; k++)
                result += (a->mx[i][k])*(b->mx[k][j]);
            m->mx[i][j] = result;
        }
    }
    return m;
}
int get_random(int min, int max){
    return rand() % (max - min + 1) + min;
}
void write_mx_to_file(FILE *file, struct matrix *a){
    fseek(file, 0, SEEK_SET);
    for (int r = 0; r < a->row_nr; r++) {
        for (int c = 0; c < a->col_nr; c++) {
            if (c > 0) fprintf(file, " ");
            fprintf(file, "%d", a->mx[r][c]);
        }
        fprintf(file, "\n");
    }
}
void get_random_mx(int rows, int cols, char *filename) {
    FILE* file = fopen(filename, "w+");
    struct matrix *m = new_matrix(rows, cols);
    for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
            m->mx[r][c] = get_random(-100, 100);
    write_mx_to_file(file, m);
    fclose(file);
}
