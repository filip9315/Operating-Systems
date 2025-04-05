#define POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t confirmed = 0;

void confirm_handler(int sig) {
    confirmed = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <catcher-pid> <mode>\n", argv[0]);
        return 1;
    }

    pid_t catcher_pid = atoi(argv[1]);
    int mode = atoi(argv[2]);

    struct sigaction sa;
    sa.sa_handler = confirm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    union sigval val;
    val.sival_int = mode;
    if (sigqueue(catcher_pid, SIGUSR1, val) == -1) {
        perror("sigqueue");
        return 1;
    }

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    while (!confirmed)
        sigsuspend(&oldmask);

    return 0;
}
