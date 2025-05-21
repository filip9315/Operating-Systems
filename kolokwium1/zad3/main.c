#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        char *filename1 = argv[1];
        int fd[2];
        pipe(fd);
        pid_t pid = fork();
        if (pid == 0)
        {
            close(fd[1]);
            execlp("sort", "sort", "--reverse", filename1, NULL);
            perror("execlp");
            exit(3);
        }
        else
        {
            close(fd[0]);
        }
    }
    else if (argc == 3)
    {
        char *filename1 = argv[1];
        char *filename2 = argv[2];
        int fd[2];

        int file_fd = open(filename2, O_WRONLY | O_CREAT | O_TRUNC, 0744);
        if (file_fd == -1) {
            perror("open");
            exit(3);
        }
        
        pipe(fd);
        pid_t pid = fork();
        if (pid == 0)
        {
            close(fd[1]);
            if (dup2(file_fd, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(3);
            }
            close(file_fd);
            execlp("sort", "sort", filename1, NULL);
            perror("execlp");
            exit(3);
        }
        else
        {
            close(fd[0]);
        }
    }
    else
        printf("Wrong number of args! \n");

    return 0;
}