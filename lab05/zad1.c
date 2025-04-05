#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

void sigusr1_handler() {
    printf("Received SIGUSR1\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "You must provide an argument.\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN);
    } else if (strcmp(argv[1], "handler") == 0) {
        struct sigaction sa;
        sa.sa_handler = sigusr1_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGUSR1, &sa, NULL);
    } else if (strcmp(argv[1], "mask") == 0) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
            perror("sigprocmask");
            exit(EXIT_FAILURE);
        }
    }

    raise(SIGUSR1);

    if (strcmp(argv[1], "mask") == 0) {
        sigset_t pending;
        if (sigpending(&pending) == -1) {
            perror("sigpending");
            exit(EXIT_FAILURE);
        }
        if (sigismember(&pending, SIGUSR1)) {
            printf("SIGUSR1 is pending\n");
        } else {
            printf("SIGUSR1 is not pending\n");
        }
    }

    return 0;
}