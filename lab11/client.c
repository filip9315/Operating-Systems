#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define ID_SIZE 32

int sockfd;
char id[ID_SIZE];

void *receiver_thread(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            printf("Disconnected from server.\n");
            exit(0);
        }
        buffer[bytes] = '\0';
        if (strncmp(buffer, "ALIVE", 5) != 0) {
            printf("%s", buffer);
        }
    }
    return NULL;
}

void handle_sigint(int sig) {
    send(sockfd, "STOP\n", 5, 0);
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <id> <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
   
    strncpy(id, argv[1], ID_SIZE - 1);
    const char *server_ip = argv[2];
    int port = atoi(argv[3]);
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    send(sockfd, id, strlen(id), 0);

    signal(SIGINT, handle_sigint);

    pthread_t tid;
    pthread_create(&tid, NULL, receiver_thread, NULL);

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        send(sockfd, buffer, strlen(buffer), 0);
    }

    return 0;
}