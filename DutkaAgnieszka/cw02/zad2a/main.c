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
#include <linux/times.h>
#include <values.h>
#include <dirent.h>
const int N=5;
const char* commands[]= {"-mtime", "-atime", "-maxdepth"};
static struct timespec initialization_time;
char * type_to_string(int d_type);
char * time_to_string(time_t time);
char *dir();
struct filter{
    bool on;
    char type;
    char modifier;
    long value;
};
struct filter* new_filter(char type);
void set_filter(struct filter *fltr, char *val);
struct settings{
    struct filter *mtime_fltr;
    struct filter *atime_fltr;
    long max_depth;
};
struct settings* new_settings();
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
void find(char *path, struct settings *sett, long depth){
    if (sett->max_depth<= depth) return;

    DIR* dir = opendir(path);
    char *next_path = calloc(PATH_MAX, sizeof(char));
    struct stat *stats = calloc(1, sizeof *stats);
    struct dirent *d;

    while ((d = readdir(dir)) != NULL) {
        snprintf(next_path, PATH_MAX, "%s/%s", path, d->d_name);
        stat(next_path, stats); // get file attributes
        if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0
        || !filter_by_time(stats->st_mtim.tv_sec, sett->mtime_fltr)
        || !filter_by_time(stats->st_atim.tv_sec, sett->atime_fltr)){
            continue;
        }

        time_t mtime = stats->st_mtim.tv_sec;
        time_t atime = stats->st_atim.tv_sec;

        printf("%s/%s | type: %s | total_links: %lu  | size: %ld bytes | atime: %s | mtime: %s\n",
                path, d->d_name, type_to_string(d->d_type), stats->st_nlink, stats->st_size,
               time_to_string(atime), time_to_string(mtime));

        if (d->d_type == DT_DIR)
            find(next_path, sett, depth-1);
    }
    free(next_path);
    closedir(dir);
}
int main(int argc, char **argv) {
    clock_gettime(CLOCK_REALTIME, &initialization_time);
    char *path = dir();
    struct settings *sett = new_settings();

    for (int nr=1; nr < argc; nr++) {
        if (strcmp(argv[nr], commands[0]) == 0) { // mtime
            char *val = argv[++nr];
            set_filter(sett->mtime_fltr, val);
        }else
            if (strcmp(argv[nr], commands[1]) == 0) { // atime
            char *val = argv[++nr];
            set_filter(sett->atime_fltr, val);
        }else
            if (strcmp(argv[nr], commands[2]) == 0) { // max depth
            sett->max_depth = strtol(argv[++nr], NULL, 10);
        }else {
            strcpy(path, argv[nr]);
        }
    }

    find(path, sett, 0);
    return 0;
}
char *dir() {
    char *cwd = calloc(PATH_MAX, sizeof(char));
    getcwd(cwd, PATH_MAX);
    return cwd;
}
struct settings* new_settings(){
    struct settings *sett = malloc(sizeof(struct settings));
    sett->mtime_fltr = new_filter('m');
    sett->atime_fltr= new_filter('a');
    sett->max_depth = LONG_MAX;
    return sett;
}
struct filter* new_filter(char type){
    struct filter *fltr = malloc(sizeof(struct filter));
    fltr->on = false;
    fltr->modifier = ' ';
    fltr->value =0;
    fltr->type = type;
    return fltr;
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

