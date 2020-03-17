#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <values.h>
#include <dirent.h>
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
const char* commands[]= {"-mtime", "-atime", "-maxdepth"};
static struct timespec initialization_time;
char * type_to_string(int d_type);
char * time_to_string(time_t time);
char *dir();
struct filter{
    bool on;
    char modifier;
    long value;
};
struct settings{
    struct filter *mtime_fltr;
    struct filter *atime_fltr;
    long max_depth;
};
void set_filter(struct filter *fltr, char *val);
struct filter mfilter = {false, ' ', 0};
struct filter afilter = {false, ' ', 0};
struct settings sett = {&mfilter, &afilter, LONG_MAX};

bool filter_by_time(time_t time, struct filter *flter){
    if(!flter->on) {return true;}
    int diff = (int) ((initialization_time.tv_sec - time) / 86400);
    switch (flter->modifier){
        case'+':
            return diff > flter->value;
        case'-':
            return diff < flter->value;
        default:
            return diff == flter->value;
    }
}
int summarize(const char *fpath, const struct stat *stats, int typeflag, struct FTW *ftwbuf){
    if (sett.max_depth < ftwbuf->level) return 0;

    time_t mtime = stats->st_mtim.tv_sec;
    time_t atime = stats->st_atim.tv_sec;
    if( !filter_by_time(atime, sett.atime_fltr) ||
        !filter_by_time(mtime, sett.mtime_fltr)){
        return 0;
    }
    printf("%s | type: %s | total_links: %lu  | size: %ld bytes | atime: %s | mtime: %s\n",
                fpath, type_to_string(typeflag), stats->st_nlink, stats->st_size,
               time_to_string(atime), time_to_string(mtime));
    return 0;
}
int main(int argc, char **argv) {
    clock_gettime(CLOCK_REALTIME, &initialization_time);
    char *path = dir();

    for (int nr=1; nr < argc; nr++) {
        if (strcmp(argv[nr], commands[0]) == 0) { // mtime
            char *val = argv[++nr];
            set_filter(sett.mtime_fltr, val);
        }else
            if (strcmp(argv[nr], commands[1]) == 0) { // atime
            char *val = argv[++nr];
            set_filter(sett.atime_fltr, val);
        }else
            if (strcmp(argv[nr], commands[2]) == 0) { // max depth
            sett.max_depth = strtol(argv[++nr], NULL, 10);
        }else {
            strcpy(path, argv[nr]);
        }
    }
    nftw(path, summarize, 0, 0);
    return 0;
}
char *dir() {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    return cwd;
}
void set_filter(struct filter *fltr, char* val) {
    fltr->on = true;
    fltr->modifier = val[0];
    fltr->value = abs((int) strtol(val, NULL, 10));
}
char * type_to_string(int d_type){
    switch (d_type) {
        case DT_BLK:
            return  "block dev";
        case DT_CHR:
            return  "char dev";
        case DT_DIR:
            return  "dir";
        case DT_FIFO:
            return  "fifo";
        case DT_LNK:
            return  "slink";
        case DT_REG:
            return  "file";
        case DT_SOCK:
            return  "sock";
        case DT_UNKNOWN:
        default:
            return "unknown";
    }
}
char * time_to_string(time_t time){
    size_t size = 256;
    char *res = calloc(size, sizeof(char));
    struct tm *local = localtime(&time);
    strftime(res, size, "%Y/%m/%d, %H:%M:%S", local);
    return res;
}

