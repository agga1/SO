#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    if(argc < 4){
        puts("expecetd arguments: \n stream_path, consumer_out_file, N");
        return 88;
    }
    char *stream_path = argv[1];
    char *consumer_out = argv[2];
    int N = atoi(argv[3]);
    char *line = calloc((size_t) N, sizeof(char));
    FILE* stream = fopen(stream_path, "r");
    FILE* file = fopen(consumer_out, "w");

    while (fgets(line, N, stream)) {
        fprintf(file, "%s", line);
    }
    fclose(file);
    fclose(stream);
    free(line);
    return 0;
}