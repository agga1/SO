#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>
#include <stdbool.h>
#include "common_func.h"
bool got_parent, got_child;
void parentsig_handler(int _) { got_parent = true; }
void childsig_handler(int _) { got_child = true; }

int main(int argc, char const *argv[])
{
    if(argc < 3){
        puts("not enough arguments, shoud be like:\n"
             "fork|exec, ignore|mask|handler|pending, only10| ");
    }
    signal(SIGUSR1, parentsig_handler);
    signal(SIGUSR2, childsig_handler);
    puts("SIG NR | CH |  P  |");
    if(argc == 4) // only SIGUSR1 = 10
    {
        printf("%7d|", 10);
        got_child = got_parent = false;
        if (fork() == 0) {
            execl("./test", "./test", argv[1], itoa(10), argv[2],  NULL);
        }
        waitpid(WAIT_ANY, NULL, WUNTRACED);
        printf("%4d|%5d|\n", got_child ? 1 : 0, got_parent ? 1 : 0);
    }
    else { // test all signals
        for (int sig = 1; sig < 23; sig++) {
            printf("%7d|", sig);
            got_child = got_parent = false;
            if (fork() == 0) {
                execl("./test", "./test", argv[1], itoa(sig), argv[2],  NULL);
            }
            waitpid(WAIT_ANY, NULL, WUNTRACED);
            printf("%4d|%5d|\n", got_child ? 1 : 0, got_parent ? 1 : 0);
        }
    }

}