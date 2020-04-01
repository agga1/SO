#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include "common.h"

bool wait_for_end = true;
bool wait_for_confirm = false;
int sender_pid;
int received = 0;
int send_mode;
int SIG1;
int SIG2;

void confirm_handler(int sig_nr, siginfo_t *siginfo, void *context){
//    puts("[C] conf received");
    wait_for_confirm = false;
}
void sig1_handler(int sig_nr, siginfo_t *siginfo, void *context){
    int nr= (send_mode==M_SIGQUEUE) ? siginfo->si_value.sival_int : 0;
    send_signal(send_mode, siginfo->si_pid, SIG1, nr);
    received ++;
}
void sig2_handler(int sig_nr, siginfo_t *siginfo, void *context){
    printf("catcher\n  received: %d \n", received);
    sender_pid = siginfo->si_pid;
    wait_for_end = false;
}
int main(int argc, char const *argv[]) {
    if(argc < 3){
        puts("not enough arguments, shoud be like:\n"
             "signal_count, send_mode");
    }
    int send_mode = str_to_mode(argv[2]);
    set_sigs(send_mode, &SIG1, &SIG2);

    printf("catcher pid [ %d ]\n", getpid());

    // TODO only for testing
//    if(fork()==0) execlp("./sender", "./sender", itoa(getppid()), argv[1], argv[2], NULL);

    struct sigaction usr1_act = {.sa_flags = SA_SIGINFO, .sa_sigaction=sig1_handler};
    sigaction(SIG1, &usr1_act, NULL);

    struct sigaction usr2_act= {.sa_flags = SA_SIGINFO, .sa_sigaction=sig2_handler};
    sigaction(SIG2, &usr2_act, NULL);

    sigset_t tmp_mask;
    sigfillset(&tmp_mask);
    sigdelset(&tmp_mask, SIG1);
    sigdelset(&tmp_mask, SIG2);

    //// receive
    while (wait_for_end){
        sigsuspend(&tmp_mask);
    }
    /// emit
    usr1_act.sa_sigaction=confirm_handler;
    sigaction(SIG1, &usr1_act, NULL);

    for(int i=0;i<received;i++){
        wait_for_confirm = true;
        send_signal(send_mode, sender_pid, SIG1, i);
        while(wait_for_confirm){};
    }
    send_signal(send_mode, sender_pid, SIG2, 0);
}