#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

double f(double x){
    return 4/(x*x + 1);
}

int main(int argc, char *argv[]){
    if (argc != 3){
        printf("Nalezy podac dwa argumenty");
        exit(EXIT_FAILURE);
    }

    int d = atoi(argv[1]);
    int n = atoi(argv[2]);

    double width = 1.0 / d;

    for (int k = 1; k <= n; ++k) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        int pipes[k][2];
        pid_t pids[k];

        for (int i = 0; i < k; ++i) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                close(pipes[i][0]);

                double local_sum = 0.0;
                int from = i * (d / k);
                int to = (i == k - 1) ? d : (i + 1) * (d / k);

                for (int j = from; j < to; ++j) {
                    double x = j * width + width / 2.0;
                    local_sum += f(x) * width;
                }

                write(pipes[i][1], &local_sum, sizeof(double));
                close(pipes[i][1]);
                exit(0);
            } else {
                close(pipes[i][1]);
                pids[i] = pid;
            }
        }

        double result = 0.0;
        for (int i = 0; i < k; ++i) {
            double partial;
            read(pipes[i][0], &partial, sizeof(double));
            close(pipes[i][0]);
            result += partial;
        }

        for (int i = 0; i < k; ++i)
            waitpid(pids[i], NULL, 0);

        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("k = %d, wynik = %.10f, czas = %.6f s\n", k, result, elapsed);
    }
}

// ./zad1 100000000000 10