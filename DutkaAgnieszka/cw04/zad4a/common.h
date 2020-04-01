#include <stdio.h>
#include <string.h>
#include <signal.h>

#define M_KILL 0
#define M_SIGQUEUE  1
#define M_SIGRT  2
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
void send_signal(int mode, int pid, int sig, union sigval *sv){
    switch (mode) {
        case M_KILL:
        case M_SIGRT:
            kill(pid, sig);
            break;
        case M_SIGQUEUE:
            sv->sival_int += 1;
            sigqueue(pid, sig, *sv);
    }
}