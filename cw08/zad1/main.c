#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LINE_BUFF 256
#define WHITESPACE " \t\r\n"
#define SIGN_MODE 0
#define BLOCK_MODE 1
#define INTER_MODE 2
int mode;
int threads_nr;
int height, width;
unsigned char **image;
int **hist_parts;

int str_to_mode(char *mode){
    if(strcmp(mode, "sign") == 0) return SIGN_MODE;
    if(strcmp(mode, "block") == 0) return BLOCK_MODE;
    if(strcmp(mode, "interleaved") == 0) return INTER_MODE;
    perror("unrecognized mode");
    exit(1);
}

__time_t elapsed_time(struct timespec *start) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    return (end.tv_sec - start->tv_sec) * 1000000 + (end.tv_nsec - start->tv_nsec) / 1000;
}

void get_signif_line(char *buffer, FILE *file) {
    fgets(buffer, LINE_BUFF, file);
    while (buffer[0] == '#' || buffer[0] == '\n')
        fgets(buffer, LINE_BUFF, file);
}

void load_pgm(char *image_filename) {
    FILE *file = fopen(image_filename, "r");
    if(file == NULL){
        perror("cannot open file");
        exit(1);
    }
    char buffer[LINE_BUFF + 1] = {0};
    get_signif_line(buffer, file); //  P2
    get_signif_line(buffer, file); // dims
    width = atoi(strtok(buffer, WHITESPACE));
    height = atoi(strtok(NULL, WHITESPACE));
//    printf("width %d, height %d\n", width, height);

    image = calloc(height, sizeof(char *));
    for (int i = 0; i < height; i++) {
        image[i] = calloc(width, sizeof(char));
    }
    get_signif_line(buffer, file); // max value (always 255)

    // !! data not necessarily  in heightXwidth format !!
    get_signif_line(buffer, file);
    char *val_str = strtok(buffer, WHITESPACE);
    for (int i = 0; i < width * height; i++) {
        if (val_str == NULL) { // get next line
            get_signif_line(buffer, file);
            val_str = strtok(buffer, WHITESPACE);
        }
        image[i / width][i % width] = atoi(val_str);
        val_str = strtok(NULL, WHITESPACE);
    }
    fclose(file);
}
int find_max(const int *array, int len){
    int max = array[0];
    for (int i = 1; i < len; i++)
        max = max < array[i] ? array[i] : max;
    return max;
}
int *join_histogram(){
    int *hist = calloc(256, sizeof(int));
    if(mode == SIGN_MODE){ // always only 1 occupied
        hist = hist_parts[0];
    } else{
        for (int i = 0; i < threads_nr; i++)
            for (int x = 0; x < 256; x++)
                hist[x] += hist_parts[i][x];
    }
    return hist;
}
void save_histogram(char *name, int height) {
    //// the bigger height, the better precision of saved histogram
    FILE *file = fopen(name, "w+");

    int *hist = join_histogram();
    int max = find_max(hist, 256);

    int width = 256;
    fprintf(file, "P2\n%d %d\n255\n", width, height);

    int heights[256] = {0}; // height of each poll
    for(int x=0;x<width;x++){
        heights[x] = (hist[x]*height)/max;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (height - y > heights[x]) fputs("0 ", file);
            else                         fputs("255 ", file);
        }
        fputs("\n", file);
    }
    fclose(file);
}
__time_t worker(int *idx);

void count_sign(int idx) {
    double part = 256.0 / threads_nr;
    int from = (int) (idx * part);
    int to = (int) ((idx + 1) * part);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (image[y][x] < to && image[y][x] >= from) {
                hist_parts[0][image[y][x]]++;
            }
        }
    }
}

void count_block(int k) {
    int part = 1+((width -1)/ threads_nr); // ceiling
    for (int x = k * part; x < (k + 1) * part; x++)
        for (int y = 0; y < height; y++)
            hist_parts[k][image[y][x]]++;
}

void count_interleaved(int k) {
    for (int x = k; x < width; x += threads_nr)
        for (int y = 0; y < height; y++)
            hist_parts[k][image[y][x]]++;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        puts("Provide 3 or 4 arguments if output_name not provided, it is generated automatically):\n"
             "threads_nr mode image_filename [output_filename]");
        return 1;
    }

    threads_nr = (int) strtol(argv[1], NULL, 10);
    mode = str_to_mode(argv[2]);
    char *filename = argv[3];
    char out_name[256];
    if(argc == 5){
        snprintf(out_name, 256, "%s", argv[4]);
    }else{
        char *tmp = calloc(strlen(filename)+1, sizeof(char));
        strcpy(tmp, filename);
        snprintf(out_name, 256, "out/%dm_%dt_%s.ascii.pgm", mode, threads_nr, strtok(tmp, "."));
    }
    load_pgm(filename);

    //// -----------start time -------------------
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    hist_parts = calloc((size_t) threads_nr, sizeof(int *));
    for (int i = 0; i < threads_nr; i++)
        hist_parts[i] = calloc(256, sizeof(int));

    pthread_t *threads = calloc((size_t) threads_nr, sizeof(pthread_t));
    int *args = calloc((size_t) threads_nr, sizeof(int));

    for (int i = 0; i < threads_nr; i++) {
        args[i] = i;
        pthread_create(&threads[i], NULL, (void *(*)(void *))worker, args + i);
    }

    for (int i = 0; i < threads_nr; i++) {
        int elapsed_time;
        pthread_join(threads[i], (void *)&elapsed_time);
        printf("thread %d took %d microseconds\n", i, elapsed_time);
    }
    //// ----------------end time --------------------
    printf("total time: %ld microseconds\n\n", elapsed_time(&start));
    save_histogram(out_name, 400);

    // free memory
    free(threads);
    free(args);
    for (int i = 0; i < threads_nr; i++) free(hist_parts[i]);
    free(hist_parts);
    for (int i = 0; i < height; i++) free(image[i]);
    free(image);
}

__time_t worker(int *idx){
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    switch (mode){
        case SIGN_MODE:
            count_sign(*idx);
            break;
        case BLOCK_MODE:
            count_block(*idx);
            break;
        case INTER_MODE:
            count_interleaved(*idx);
            break;
        default:
            perror("unrecognized mode");
            exit(1);
    }
    return elapsed_time(&start);
}