#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "common.h"

bool wait = true;
int received =0;
int send_mode;
int SIG1 = SIGUSR1;
int SIG2 = SIGUSR2;

void sig1_handler(int sig_nr, siginfo_t *siginfo, void *context){
    received ++;
}
void sig2_handler(int sig_nr, siginfo_t *siginfo, void *context){
    wait = false;
    printf("catcher:\n  received %d \n", received);
    int pid = siginfo->si_pid;
    union sigval sv;
    sv.sival_int = 0;
    for(int i=0;i<received;i++)
        send_signal(send_mode, pid, SIG1, &sv);
    send_signal(send_mode, pid, SIG2, &sv);
}
int main(int argc, char const *argv[]) {
    if(argc < 3){
        puts("not enough arguments, shoud be like:\n"
             "signal_count, send_mode  (catcher automatically runs sender)");
    }
    int send_mode = str_to_mode(argv[2]);
    if(send_mode==M_SIGRT) {
        SIG1 = SIGRTMIN;
        SIG2 = SIGRTMIN+1;
    }
    printf("catcher pid [ %d ]\n", getpid());

    // TODO only for testing (disable if you want to run sender on your own)
    if(fork()==0) execlp("./sender", "./sender", itoa(getppid()), argv[1], argv[2], NULL);

    struct sigaction usr1_act = {.sa_flags = SA_SIGINFO, .sa_sigaction=sig1_handler};
    sigaction(SIG1, &usr1_act, NULL);

    struct sigaction usr2_act= {.sa_flags = SA_SIGINFO, .sa_sigaction=sig2_handler};
    sigaction(SIG2, &usr2_act, NULL);

    sigset_t tmp_mask;
    sigfillset(&tmp_mask);
    sigdelset(&tmp_mask, SIG1);
    sigdelset(&tmp_mask, SIG2);

    while (wait){
        sigsuspend(&tmp_mask);
    }
}