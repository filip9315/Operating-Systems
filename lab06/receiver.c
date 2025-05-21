#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

double f(double x) {
    return 4.0 / (x * x + 1.0);
}

int main() {
    mkfifo("pipe1", 0666);
    mkfifo("pipe2", 0666);

    while (1) {
        int dummy_fd = open("pipe1", O_WRONLY | O_NONBLOCK);
        if (dummy_fd < 0) {}

        int fd1 = open("pipe1", O_RDONLY);
        if (fd1 < 0) {
            perror("open pipe1");
            sleep(1);
            continue;
        }
        double a, b;
        ssize_t r1 = read(fd1, &a, sizeof(double));
        ssize_t r2 = read(fd1, &b, sizeof(double));
        close(fd1);

        if (dummy_fd >= 0) close(dummy_fd);

        if (r1 != sizeof(double) || r2 != sizeof(double)) continue;

        int N = 100000000;
        double width = (b - a) / N;
        double sum = 0.0;

        for (int i = 0; i < N; ++i) {
            double x = a + (i + 0.5) * width;
            sum += f(x) * width;
        }

        int fd2 = open("pipe2", O_WRONLY);
        if (fd2 < 0) {
            perror("open pipe2");
            continue;
        }
        write(fd2, &sum, sizeof(double));
        close(fd2);

        printf("Całka z przedziału [%.2f, %.2f] = %.10f\n", a, b, sum);
    }

    return 0;
}
