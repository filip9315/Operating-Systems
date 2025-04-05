#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    pid_t child_pid;
    int n = atoi(argv[1]);

    for (int i = 0; i < n; i++) {
        if ((child_pid = fork()) == 0) {
            printf("Child: ppid:%d, pid:%d\n", (int)getppid(), (int)getpid());
            exit(0);
        }
    }

    while (wait(NULL) > 0);

    printf("Parent: ppid:%d, pid:%d\n", (int)getppid(), (int)getpid());
    printf("argv[1]: %s\n", argv[1]);
    return 0;
}