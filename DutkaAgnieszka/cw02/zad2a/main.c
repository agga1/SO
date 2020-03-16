#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <sys/resource.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <values.h>

const int N=5;
const char* commands[]= {"-mtime", "-atime", "-maxdepth"};
char *dir();
struct filter{
    bool on;
    char modifier;
    long value;
};
struct filter* new_filter();
struct settings{
    struct filter *mtime_fltr;
    struct filter *atime_fltr;
    long max_depth;
};
struct settings* new_settings();
int main(int argc, char **argv) {
    if(argc<2){
        printf("no file specified");
        return 1;
    }
    int nr=1;
    char *filename = argv[nr++];
    char *path = dir();
    struct settings *sett = new_settings();

    for (; nr < argc; nr++) {
        if (strcmp(argv[nr], commands[0]) == 0) { // mtime
            char *val = argv[++nr];
            sett->mtime_fltr->on = true;
            sett->mtime_fltr->modifier = val[0];
            sett->mtime_fltr->value = strtol(val, NULL, 10);
        }else if (strcmp(argv[nr], commands[1]) == 0) { // atime
            char *val = argv[++nr];
            sett->atime_fltr->on = true;
            sett->atime_fltr->modifier = val[0];
            sett->atime_fltr->value = strtol(val, NULL, 10);
        }else if (strcmp(argv[nr], commands[2]) == 0) { // max depth
            sett->max_depth = strtol(argv[++nr], NULL, 10);
        }else {
            strcpy(path, argv[nr]);
        }
    }
    return 0;
}
char *dir() {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    return cwd;
}
struct settings* new_settings(){
    struct settings *sett = malloc(sizeof(struct settings));
    sett->mtime_fltr = new_filter();
    sett->atime_fltr= new_filter();
    sett->max_depth = LONG_MAX;
    return sett;
}
struct filter* new_filter(){
    struct filter*fltr = malloc(sizeof(struct filter));
    fltr->on = false;
    fltr->modifier = ' ';
    fltr->value =0;
    return fltr;
}