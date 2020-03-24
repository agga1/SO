#define _XOPEN_SOURCE 500
#define LINE_BUFF 4096
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

struct matrix {
    int** mx;
    int row_nr;
    int col_nr;
};
struct matrix *new_matrix(int ** val, int rows, int cols){
    struct matrix *m = malloc(sizeof(struct matrix));
    m->row_nr = rows;
    m->col_nr = cols;
    m->mx = val;
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
    char line[LINE_BUFF];
    // get row and col nr
    int rows=0, cols=0;
    if(fgets(line, LINE_BUFF, file) != NULL){
        cols = get_col_nr(line);
        rows++;
        while (fgets(line, LINE_BUFF, file) != NULL)
            rows ++;
    }

    fseek(file, 0, SEEK_SET); // rewind to the beginning
    int** values = calloc(rows, sizeof(int*));
    for (int y = 0; y < rows; y++) values[y] = calloc(cols, sizeof(int));

    int x_curr, y_curr = 0;
    while (fgets(line, LINE_BUFF, file) != NULL) {
        x_curr = 0;
        char* number = strtok(line, " ");
        while (number != NULL) {
            values[y_curr][x_curr++] = atoi(number);
            number = strtok(NULL, " ");
        }
        y_curr++;
    }
    fclose(file);
    return new_matrix(values, rows, cols);
}

void free_mx(struct matrix *m) {
    for (int y = 0; y < m->row_nr; y++) free(m->mx[y]);
    free(m->mx);
}

struct matrix* dot(struct matrix *a, struct matrix *b){

//    struct matrix *matrix1 = new_matrix(a->row_nr)
    int** matrix = calloc((size_t) a->row_nr, sizeof(int*));
    for (int y = 0; y < a->row_nr; y++) matrix[y] = calloc((size_t) b->col_nr, sizeof(int));

    for (int i = 0; i < a->row_nr; i++) {
        for (int j = 0; j < b->col_nr; j++) {
            int result = 0;
            for(int k=0; k< a->col_nr; k++){
                result += (a->mx[i][k]*b->mx[k][j]);
            }
            matrix[i][j] = result;
        }
    }
    return new_matrix(matrix, a->row_nr, b->col_nr);
}
int get_random(int min, int max){
    return rand() % (max - min + 1) + min;
}
void generate_matrix(int rows, int cols, char* filename) {
    FILE* file = fopen(filename, "w+");

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (x > 0) fprintf(file, " ");
            fprintf(file, "%d", get_random(-100, 100));
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

void write_mx_to_file(FILE *file, struct matrix *a){
    fseek(file, 0, SEEK_SET);
    for (int y = 0; y < a->row_nr; y++) {
        for (int x = 0; x < a->col_nr; x++) {
            if (x > 0) {
                fprintf(file, " ");
            }
            fprintf(file, "%d", a->mx[y][x]);
        };
        fprintf(file, "\n");
    }
}
