#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include "common.h"
bool wait_for_end = true;
bool wait_for_confirm=false;

int received =0;
int sig_count=0;
int SIG1 = SIGUSR1;
int SIG2 = SIGUSR2;
int send_mode;

void confirm_handler(int _, siginfo_t *siginfo, void *context){
    puts("[S] conf received");
    wait_for_confirm = false;
}
void sig1_handler(int _, siginfo_t *siginfo, void *context){
    int nr= (send_mode==M_SIGQUEUE) ? siginfo->si_value.sival_int : 0;
    send_signal(send_mode, siginfo->si_pid, SIG1, nr);
    received ++;
}
void sig2_handler(int _, siginfo_t *siginfo, void *context){
    printf("sender: \n  received: %d\n  expected: %d\n", received, sig_count);
    if(send_mode==M_SIGQUEUE)
        printf("[QueueOpt] catcher received %d", siginfo->si_value.sival_int);
    wait_for_end = false;
}
int main(int argc, char const *argv[])
{
    if(argc < 4){
        puts("not enough arguments, shoud be like:\n"
             "catcher PID, signal_count, send_mode");
    }

    int cpid = atoi(argv[1]);
    sig_count = atoi(argv[2]);
    send_mode = str_to_mode(argv[3]);

    if(send_mode==M_SIGRT) {
        SIG1 = SIGRTMIN;
        SIG2 = SIGRTMIN+1;
    }

    struct sigaction usr1_act = {.sa_flags = SA_SIGINFO, .sa_sigaction=confirm_handler};
    sigaction(SIG1, &usr1_act, NULL);

    struct sigaction usr2_act= {.sa_flags = SA_SIGINFO, .sa_sigaction=sig2_handler};
    sigaction(SIG2, &usr2_act, NULL);

    //// emit signals
    for(int i=0;i<sig_count;i++){
        wait_for_confirm=true;
        send_signal(send_mode, cpid, SIG1, i);
        while(wait_for_confirm){};
    }
    usr1_act.sa_sigaction=sig1_handler;
    sigaction(SIG1, &usr1_act, NULL);
    puts("handler changed");
    send_signal(send_mode, cpid, SIG2, sig_count);
    while(wait_for_end){}
    return 0;
}
