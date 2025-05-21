#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>

#define SERVER_KEY 0x1234
#define MAX_CLIENTS 10
#define MAX_TEXT 256

typedef enum { INIT = 1, MESSAGE = 2 } msg_type;

typedef struct {
    long mtype;
    key_t client_queue_key;
    int client_id;
    char text[MAX_TEXT];
} message;

int client_queues[MAX_CLIENTS] = {0};

int main() {
    int server_qid = msgget(SERVER_KEY, IPC_CREAT | 0666);
    if (server_qid == -1) {
        perror("msgget");
        exit(1);
    }

    int client_count = 0;
    message msg;

    while (1) {
        if (msgrcv(server_qid, &msg, sizeof(message) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            continue;
        }

        if (msg.mtype == INIT) {
            if (client_count >= MAX_CLIENTS) {
                fprintf(stderr, "Max clients reached\n");
                continue;
            }

            int client_id = client_count++;
            client_queues[client_id] = msgget(msg.client_queue_key, 0);

            message reply = { .mtype = INIT, .client_id = client_id };
            msgsnd(client_queues[client_id], &reply, sizeof(message) - sizeof(long), 0);
        }

        if (msg.mtype == MESSAGE) {
            for (int i = 0; i < client_count; i++) {
                if (i != msg.client_id) {
                    msgsnd(client_queues[i], &msg, sizeof(message) - sizeof(long), 0);
                }
            }
        }
    }

    return 0;
}