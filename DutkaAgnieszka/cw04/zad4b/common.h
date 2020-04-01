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
void send_signal(int mode, int pid, int sig, int nr){
    switch (mode) {
        case M_KILL:
        case M_SIGRT:
            kill(pid, sig);
            break;
        case M_SIGQUEUE:
        {
            union sigval sv = {.sival_int=nr};
            sigqueue(pid, sig, sv);
        }

    }
}