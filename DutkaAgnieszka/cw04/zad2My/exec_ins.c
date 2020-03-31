#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
void fail_handler(int _) { _exit(0); }

int main(int argc, char **argv)
{
    int sig_nr = atoi(argv[1]);
    int ppid = atoi(argv[2]);

    switch(argv[3][0]){
        case 'i':
            raise(sig_nr);
            kill(ppid, SIGUSR2);
            break;
        case 'm':
            signal(sig_nr, fail_handler); // should be masked
            raise(sig_nr);
            kill(ppid, SIGUSR2);
            break;
        case 'p':
        {
            sigset_t mask;
            sigpending(&mask);
            if (sigismember(&mask, sig_nr))
                kill(ppid, SIGUSR2);
            break;
        }
        default:
            puts("argument unrecognized");
    }
}