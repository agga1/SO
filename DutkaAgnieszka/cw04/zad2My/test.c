#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <stdbool.h>
#include "common_func.h"
#define EXEC_PROG "./exec_ins"

bool is_exec = false;
static int sig_nr;
static pid_t main_id;

int succ_sig = SIGUSR2;
void fail_handler(int _) { _exit(0); }
void succ_handler(int _) { kill(main_id, succ_sig); }

void handle_t()
{
    signal(sig_nr, succ_handler);
    if (fork() == 0)
        raise(sig_nr);
    else{
        waitpid(WAIT_ANY, NULL, WUNTRACED);
        succ_sig = SIGUSR1;
        raise(sig_nr);
    }
}

void ignore_t()
{
    signal(sig_nr, SIG_IGN);
    if (!is_exec && fork() == 0) { // CHILD PROCESS
            raise(sig_nr);
            kill(main_id, SIGUSR2);
    }
    else{  // PARENT PROCESS
        waitpid(WAIT_ANY, NULL, WUNTRACED);
        raise(sig_nr);
        kill(main_id, SIGUSR1);
    }
    if (is_exec) execl(EXEC_PROG, EXEC_PROG, itoa(sig_nr), itoa(main_id), "i", NULL);
}

void mask_t()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig_nr);
    sigprocmask(SIG_SETMASK, &mask, NULL);
    signal(sig_nr, fail_handler); // sig_nr should not be received
    if (!is_exec && fork() == 0) { // fork version
            raise(sig_nr);
            kill(main_id, SIGUSR2);
    }
    else{
        waitpid(WAIT_ANY, NULL, WUNTRACED);
        raise(sig_nr);
        kill(main_id, SIGUSR1);
    }
    if (is_exec) execl(EXEC_PROG, EXEC_PROG, itoa(sig_nr), itoa(main_id), "m", NULL);
}

void pending_t()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, sig_nr);
    sigprocmask(SIG_SETMASK, &mask, NULL);
    raise(sig_nr);
    sigset_t pending_sigs;

    if (!is_exec && fork() == 0) { // fork version
            sigpending(&pending_sigs);
            if (sigismember(&pending_sigs, sig_nr)) kill(main_id, SIGUSR2); // success, pending signal found
    }
    else{
        waitpid(WAIT_ANY, NULL, WUNTRACED);
        sigpending(&pending_sigs);
        if (sigismember(&pending_sigs, sig_nr)) kill(main_id, SIGUSR1); // success, pending signal found
    }
    if (is_exec) execl(EXEC_PROG, EXEC_PROG, itoa(sig_nr), itoa(main_id), "p", NULL);
}

int main(int argc, char const *argv[])
{
    main_id = getppid();

    is_exec = strcmp(argv[1], "exec") == 0;
    sig_nr = atoi(argv[2]);

    switch(argv[3][0]){
        case 'i':
            ignore_t();
            break;
        case 'm':
            mask_t();
            break;
        case 'p':
            pending_t();
            break;
        case 'h':
            handle_t();
            break;
        default:
            puts("argument unrecognized");
    }
    return 0;
}


