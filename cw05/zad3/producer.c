#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if(argc < 4){
        puts("expected arguments: \n"
             "stream_path, res_file, N");
        return 1;
    }
    char *stream_path = argv[1];
    char *res_file = argv[2];
    int N = atoi(argv[3]);
    char *line = calloc((size_t) N, sizeof(char));
    FILE* stream = fopen(stream_path, "w");
    FILE* file = fopen(res_file, "r");

    while (fgets(line, N, file)) {
        fprintf(stream, "#%d#%s\n", getpid(), line);
        sleep(1);
    }

    fclose(file);
    fclose(stream);
    free(line);
    return 0;
}