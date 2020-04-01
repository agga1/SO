#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

bool wait = true;
void alarm_handler(int _, siginfo_t *siginfo, void *cont){
    printf("[ sigalarm handler ]\n");
    printf(" timerid: %d\n", siginfo->si_timerid);
    wait = false;
}

void child_handler(int _, siginfo_t *siginfo, void *cont){
    printf("[ sigchld handler ]\n");
    printf(" pid: %d\n uid: %d\n status: %d\n", siginfo->si_pid, siginfo->si_uid, siginfo->si_status);
    wait = false;
}

void queue_handler(int _, siginfo_t *siginfo, void *cont){
    printf("[ siqusr1 handler ]\n");
    printf(" int: %d\n", siginfo->si_int);
    wait = false;
}

int main(int argc, char** argv){
    if (argc != 2) {
        fprintf(stderr, "wrong nr of arguments, must be like\n"
                        "sigchld|sigusr1|sigalarm");
        return -1;
    }

    struct sigaction act = {.sa_flags= SA_SIGINFO};
    if(strcmp(argv[1], "sigchld") == 0){
        act.sa_sigaction = child_handler;
        sigaction(SIGCHLD, &act, NULL);
        if(fork() == 0) exit(5);
    } else if(strcmp(argv[1], "sigalarm") == 0){
        act.sa_sigaction = alarm_handler;
        sigaction(SIGALRM, &act, NULL);
        alarm(1);
    } else if(strcmp(argv[1], "sigusr1") == 0){
        act.sa_sigaction = queue_handler;
        sigaction(SIGUSR1, &act, NULL);
        union sigval value = {.sival_int = 40};
        sigqueue(getpid(), SIGUSR1, value);
    } else{
        puts("argument unrecognized");
    }

    while(wait){}
    return 0;
}