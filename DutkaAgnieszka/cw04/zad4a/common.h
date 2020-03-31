#include <stdio.h>
#include <string.h>
#define M_KILL 0
#define M_SIGQUEUE  1
#define M_SIGRT  2
int k =5;
char *itoa(int i) {
    static char str[12];
    sprintf(str, "%i", i);
    return strdup(str);
}
int str_to_mode(const char *str){
    if(strcmp(str, "kill")==0)
        return M_KILL;
    if(strcmp(str, "sigqueue")==0)
        return M_SIGQUEUE;
    if(strcmp(str, "sigrt")==0)
        return M_SIGRT;
    return -1;
}