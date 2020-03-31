#define XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void handler( int sig_nr, siginfo_t *siginfo, void *context){
    printf("siginfo:\n signal nr %d\n si_uid: %u\n si_code: %u\n si_addr:%p\n",
            siginfo->si_signo, siginfo->si_uid, siginfo->si_code, siginfo->si_addr);
}
int main(int argc, char const *argv[]) {
    if (argc!= 2){
        printf("provide signal nr");
        return -1;
    }
    int sig_nr = atoi(argv[1]);
    struct sigaction act = {.sa_flags = SA_SIGINFO, .sa_sigaction = handler};
    sigemptyset(&act.sa_mask);

    sigaction(sig_nr, &act, NULL);
    raise(sig_nr);
    return 0;
}