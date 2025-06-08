#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define ID_SIZE 32

typedef struct {
    int sockfd;
    char id[ID_SIZE];
    int active;
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_client(int sockfd, const char *msg) {
    send(sockfd, msg, strlen(msg), 0);
}

void broadcast(const char *sender, const char *msg) {
    time_t now = time(NULL);
    char out[BUFFER_SIZE];
    snprintf(out, sizeof(out), "%s %s: %s", ctime(&now), sender, msg);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            send_to_client(clients[i].sockfd, out);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_one(const char *sender, const char *target_id, const char *msg) {
    time_t now = time(NULL);
    char out[BUFFER_SIZE];
    snprintf(out, sizeof(out), "[%s] %s -> %s: %s", ctime(&now), sender, target_id, msg);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].id, target_id) == 0) {
            send_to_client(clients[i].sockfd, out);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int idx) {
    close(clients[idx].sockfd);
    clients[idx].active = 0;
    memset(clients[idx].id, 0, ID_SIZE);
}

void *client_handler(void *arg) {
    int idx = *(int *)arg;
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes = recv(clients[idx].sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            remove_client(idx);
            break;
        }
        buffer[bytes] = '\0';

        if (strncmp(buffer, "LIST", 4) == 0) {
            pthread_mutex_lock(&clients_mutex);
            char list[BUFFER_SIZE] = "Active clients:\n";
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active) {
                    strcat(list, clients[i].id);
                    strcat(list, "\n");
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            send_to_client(clients[idx].sockfd, list);
        } else if (strncmp(buffer, "2ALL ", 5) == 0) {
            broadcast(clients[idx].id, buffer + 5);
        } else if (strncmp(buffer, "2ONE ", 5) == 0) {
            char target[ID_SIZE];
            sscanf(buffer + 5, "%s", target);
            char *msg_start = strchr(buffer + 5, ' ') + 1;
            send_to_one(clients[idx].id, target, msg_start);
        } else if (strncmp(buffer, "STOP", 4) == 0) {
            remove_client(idx);
            break;
        } else if (strncmp(buffer, "ALIVE", 5) == 0) {
            // ignore, just a ping response
        }
    }
    return NULL;
}

void *alive_checker(void *arg) {
    while (1) {
        sleep(10);
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                if (send(clients[i].sockfd, "ALIVE\n", 6, 0) <= 0) {
                    remove_client(i);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    pthread_t alive_thread;
    pthread_create(&alive_thread, NULL, alive_checker, NULL);

    printf("Server running on %s:%d\n", ip, port);

    while (1) {
        int client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        char id[ID_SIZE] = {0};
        recv(client_sock, id, ID_SIZE - 1, 0);

        pthread_mutex_lock(&clients_mutex);
        int idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                clients[i].sockfd = client_sock;
                strncpy(clients[i].id, id, ID_SIZE - 1);
                clients[i].active = 1;
                idx = i;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (idx != -1) {
            pthread_t tid;
            pthread_create(&tid, NULL, client_handler, &idx);
        } else {
            send(client_sock, "Server full\n", 12, 0);
            close(client_sock);
        }
    }

    close(server_fd);
    return 0;
}