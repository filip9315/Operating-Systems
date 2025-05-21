#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_QUEUE 10
#define NUM_USERS 2
#define NUM_PRINTERS 1

typedef struct {
    char tasks[MAX_QUEUE][10];
    int front;
    int rear;
    int count;
} SharedMemory;

void init_semaphores(int semid) {
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl mutex");
        exit(1);
    }
    arg.val = MAX_QUEUE;
    if (semctl(semid, 1, SETVAL, arg) == -1) {
        perror("semctl empty");
        exit(1);
    }
    arg.val = 0;
    if (semctl(semid, 2, SETVAL, arg) == -1) {
        perror("semctl full");
        exit(1);
    }
}

int main() {
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, sizeof(SharedMemory), 0666 | IPC_CREAT);
    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);

    shm->front = 0;
    shm->rear = 0;
    shm->count = 0;

    int semid = semget(key, 3, 0666 | IPC_CREAT);
    init_semaphores(semid);

    // Procesy użytkowników
    for (int i = 0; i < NUM_USERS; i++) {
        if (fork() == 0) {
            srand(time(NULL) ^ getpid());
            while (1) {
                char task[10];
                for (int j = 0; j < 10; j++) task[j] = 'a' + rand() % 26;
                
                struct sembuf sops;
                
                sops.sem_num = 1; sops.sem_op = -1; sops.sem_flg = 0;
                semop(semid, &sops, 1);

                sops.sem_num = 0; sops.sem_op = -1;
                semop(semid, &sops, 1);

                printf("User %d dodaje do kolejki: %s\n", i + 1, task);
                fflush(stdout);
                memcpy(shm->tasks[shm->rear], task, 10);
                shm->rear = (shm->rear + 1) % MAX_QUEUE;
                shm->count++;

                sops.sem_num = 0; sops.sem_op = 1;
                semop(semid, &sops, 1);

                sops.sem_num = 2; sops.sem_op = 1;
                semop(semid, &sops, 1);

                sleep(rand() % 3 + 1);
            }
        }
    }

    // Procesy drukarek
    for (int i = 0; i < NUM_PRINTERS; i++) {
        if (fork() == 0) {
            int printer_num = i + 1;
            while (1) {
                struct sembuf sops;
                sops.sem_num = 2; sops.sem_op = -1; sops.sem_flg = 0;
                semop(semid, &sops, 1);

                sops.sem_num = 0; sops.sem_op = -1;
                semop(semid, &sops, 1);

                char task[10];
                memcpy(task, shm->tasks[shm->front], 10);
                shm->front = (shm->front + 1) % MAX_QUEUE;
                shm->count--;

                sops.sem_num = 0; sops.sem_op = 1;
                semop(semid, &sops, 1);

                sops.sem_num = 1; sops.sem_op = 1;
                semop(semid, &sops, 1);

                for (int j = 0; j < 10; j++) {
                    printf("Printer %d: %c\n", printer_num, task[j]);
                    fflush(stdout);
                    sleep(1);
                }
            }
        }
    }

    while (1) pause();
    return 0;
}