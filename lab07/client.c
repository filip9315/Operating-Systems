#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SERVER_KEY 0x1234
#define MAX_TEXT 256

typedef enum { INIT = 1, MESSAGE = 2 } msg_type;

typedef struct {
    long mtype;
    key_t client_queue_key;
    int client_id;
    char text[MAX_TEXT];
} message;

int main() {
    key_t client_key = ftok(".", getpid());
    int client_qid = msgget(client_key, IPC_CREAT | 0666);

    int server_qid = msgget(SERVER_KEY, 0);

    message init_msg = { .mtype = INIT, .client_queue_key = client_key };
    msgsnd(server_qid, &init_msg, sizeof(message) - sizeof(long), 0);

    message response;
    msgrcv(client_qid, &response, sizeof(message) - sizeof(long), INIT, 0);
    int client_id = response.client_id;

    if (fork() == 0) {
        while (1) {
            message msg;
            if (msgrcv(client_qid, &msg, sizeof(message) - sizeof(long), 0, 0) > 0) {
                printf("Client %d: %s\n", msg.client_id, msg.text);
            }
        }
    } else {
        while (1) {
            message msg = { .mtype = MESSAGE, .client_id = client_id };
            fgets(msg.text, MAX_TEXT, stdin);
            msgsnd(server_qid, &msg, sizeof(message) - sizeof(long), 0);
        }
    }

    return 0;
}