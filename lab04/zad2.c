#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int global = 0;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "1 argument must be provided.");
        return 1;
    }

    printf("Program name: %s\n", argv[0]);

    int local = 0;
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork failed");
        return 1;
    } else if (child_pid == 0) {
        printf("Child process\n");
        global++;
        local++;
        printf("Child pid = %d, parent pid = %d\n", getpid(), getppid());
        printf("Child's local = %d, child's global = %d\n", local, global);
        execl("/bin/ls", "ls", argv[1], NULL);
        return 1;
    } else {
        int status;
        waitpid(child_pid, &status, 0);
        printf("Parent process\n");
        printf("Parent pid = %d, child pid = %d\n", getpid(), child_pid);
        printf("Child exit code: %d\n", WEXITSTATUS(status));
        printf("Parent's local = %d, parent's global = %d\n", local, global);
        return WEXITSTATUS(status);
    }
}