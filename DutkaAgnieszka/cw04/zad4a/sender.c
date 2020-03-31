#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include "common.h"
bool wait = true;
int received =0;
int sig_count=0;
int SIG1 = SIGUSR1;
int SIG2 = SIGUSR2;

void sig1_handler(int _, siginfo_t *siginfo, void *context){
    received ++;
}
void sig2_handler(int _, siginfo_t *siginfo, void *context){
    printf("sender: \n  received: %d\n  expected: %d\n", received, sig_count);
    wait = false;
}
int main(int argc, char const *argv[])
{
    if(argc < 4){
        puts("not enough arguments, shoud be like:\n"
             "catcher PID, signal_count, send_mode");
    }

    int cpid = atoi(argv[1]);
    sig_count = atoi(argv[2]);
    int send_mode = str_to_mode(argv[3]);

    if(send_mode==M_SIGRT) {
        SIG1 = SIGRTMIN;
        SIG2 = SIGRTMIN+1;
    }

    struct sigaction usr1_act = {.sa_flags = SA_SIGINFO, .sa_sigaction=sig1_handler};
    sigaction(SIG1, &usr1_act, NULL);

    struct sigaction usr2_act= {.sa_flags = SA_SIGINFO, .sa_sigaction=sig2_handler};
    sigaction(SIG2, &usr2_act, NULL);

    //// emit signals
    union sigval nr;
    nr.sival_int = 0;
    for(int i=0;i<sig_count;i++){
        switch (send_mode) {
            case M_KILL:
            case M_SIGRT:
                kill(cpid, SIG1);
                break;
            case M_SIGQUEUE:
                nr.sival_int = i;
                sigqueue(cpid, SIG1, nr);
        }
    }
    if(send_mode != M_SIGQUEUE) kill(cpid, SIG2);
    else sigqueue(cpid, SIG2, nr);
    while(wait){}
    return 0;
}
