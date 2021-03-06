#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <signal.h>
#include <stdbool.h>

bool running = 1;

void sigstp_handler(int _) {
    if (running) printf("\nWaiting for\n CTRL+Z -> continue running \n CTRL+C -> end program \n");
    running = !running;
}

void sigint_handler(int _) {
    printf("\nSignal SIGINT received, ending program\n");
    exit(EXIT_SUCCESS);
}
int main(int argc, char const *argv[])
{
    // handling sigstp
    struct sigaction sigstp_action;
    sigstp_action.sa_handler = sigstp_handler;
    sigstp_action.sa_flags = 0;
    sigemptyset(&sigstp_action.sa_mask);
    sigaction(SIGTSTP, &sigstp_action, NULL);

    // handle sigint
    signal(SIGINT, sigint_handler);
    while (1) {
        if (running) system("ls");
        sleep(1);
    }
}