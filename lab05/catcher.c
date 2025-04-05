#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

volatile sig_atomic_t received_count = 0;
volatile sig_atomic_t current_mode = 0;
pid_t sender_pid = 0;

void ctrlc_ignore(int sig) {}
void ctrlc_handler(int sig) {
    printf("Wciśnięto CTRL+C\n");
}

void handle_usr1(int sig, siginfo_t *info, void *ucontext) {
    sender_pid = info->si_pid;
    int mode = info->si_value.sival_int;

    if (mode == 1) {
        current_mode = 1;
        received_count++;
        printf("Liczba żądań zmiany trybu: %d\n", received_count);
    } else if (mode == 2) {
        current_mode = 2;
    } else if (mode == 3) {
        current_mode = 3;
        signal(SIGINT, SIG_IGN);
    } else if (mode == 4) {
        current_mode = 4;
        signal(SIGINT, ctrlc_handler);
    } else if (mode == 5) {
        current_mode = 5;
        exit(0);
    }

    union sigval val = {0};
    sigqueue(sender_pid, SIGUSR1, val);
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = handle_usr1;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);

    signal(SIGINT, SIG_DFL);

    printf("Catcher PID: %d\n", getpid());

    while (1) {
        if (current_mode == 2) {
            for (int i = 1; current_mode == 2; ++i) {
                printf("%d\n", i);
                sleep(1);
            }
        } else {
            pause();
        }
    }
}
