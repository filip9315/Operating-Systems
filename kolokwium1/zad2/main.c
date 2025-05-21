#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int pipefd[2];

void sighandler(int, siginfo_t *, void *);

void sighandler(int sig, siginfo_t *info, void *context) {
    (void)info;
    (void)context;
    if (sig == SIGUSR1) {
        int val;
        if (read(pipefd[0], &val, sizeof(int)) == sizeof(int)) {
            printf("Received SIGUSR1 with value: %d\n", val);
        } else {
            puts("Received SIGUSR1 but failed to read value");
        }
    }
}

int main(int argc, char* argv[]) {

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = &sighandler;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&action.sa_mask);

    int child = fork();
    if(child == 0) {
        // Proces potomny

        close(pipefd[1]);

        sigset_t set;
        sigemptyset(&set);
        sigdelset(&set, SIGUSR1);
        sigprocmask(SIG_SETMASK, &set, NULL);

        sigaction(SIGUSR1, &action, NULL);

        printf("Child process waiting for SIGUSR1...\n");
        pause();
    }
    else {
        close(pipefd[0]);

        int val = atoi(argv[1]);
        if (write(pipefd[1], &val, sizeof(int)) != sizeof(int)) {
            perror("write");
            return 1;
        }
        
        sleep(1);
        if (kill(child, atoi(argv[2])) == -1) {
            perror("kill");
            return 1;
        }

    }

    return 0;
}