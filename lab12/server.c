#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define MAX_CLIENTS 50
#define MAX_NAME_LEN 32
#define MAX_MSG_LEN 512
#define BUFFER_SIZE 1024

typedef struct {
    char name[MAX_NAME_LEN];
    struct sockaddr_in addr;
    time_t last_alive;
    int active;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
int server_socket;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void cleanup_and_exit(int sig) {
    printf("\nZamykanie serwera...\n");
    close(server_socket);
    exit(0);
}

int find_client_by_name(const char* name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int find_client_by_addr(struct sockaddr_in* addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && 
            clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

int add_client(const char* name, struct sockaddr_in* addr) {
    pthread_mutex_lock(&clients_mutex);
    
    // Sprawdź czy klient już istnieje
    int existing = find_client_by_name(name);
    if (existing != -1) {
        // Aktualizuj istniejącego klienta
        clients[existing].addr = *addr;
        clients[existing].last_alive = time(NULL);
        pthread_mutex_unlock(&clients_mutex);
        return existing;
    }
    
    // Znajdź wolne miejsce
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            strcpy(clients[i].name, name);
            clients[i].addr = *addr;
            clients[i].last_alive = time(NULL);
            clients[i].active = 1;
            client_count++;
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return -1; // Brak miejsca
}

void remove_client(int index) {
    pthread_mutex_lock(&clients_mutex);
    if (index >= 0 && index < MAX_CLIENTS && clients[index].active) {
        printf("Usuwanie klienta: %s\n", clients[index].name);
        clients[index].active = 0;
        memset(&clients[index], 0, sizeof(Client));
        client_count--;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_message(struct sockaddr_in* addr, const char* message) {
    sendto(server_socket, message, strlen(message), 0, 
           (struct sockaddr*)addr, sizeof(struct sockaddr_in));
}

void broadcast_message(const char* sender, const char* message, int sender_index) {
    char full_msg[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    
    snprintf(full_msg, sizeof(full_msg), "[%s] %s: %s", timestamp, sender, message);
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && i != sender_index) {
            send_message(&clients[i].addr, full_msg);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_private_message(const char* sender, const char* recipient, const char* message) {
    int recipient_index = find_client_by_name(recipient);
    if (recipient_index == -1) {
        return;
    }
    
    char full_msg[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    
    snprintf(full_msg, sizeof(full_msg), "[%s] %s (prywatnie): %s", timestamp, sender, message);
    
    pthread_mutex_lock(&clients_mutex);
    if (clients[recipient_index].active) {
        send_message(&clients[recipient_index].addr, full_msg);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_client_list(struct sockaddr_in* addr) {
    char list_msg[BUFFER_SIZE] = "Aktywni klienci:\n";
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(list_msg, "- ");
            strcat(list_msg, clients[i].name);
            strcat(list_msg, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    if (strlen(list_msg) == strlen("Aktywni klienci:\n")) {
        strcpy(list_msg, "Brak aktywnych klientów");
    }
    
    send_message(addr, list_msg);
}

void* alive_checker(void* arg) {
    char alive_msg[] = "ALIVE";
    
    while (1) {
        sleep(10); // Sprawdzaj co 10 sekund
        
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active) {
                time_t now = time(NULL);
                if (now - clients[i].last_alive > 10) { // 10 sekund timeout
                    printf("Klient %s nie odpowiada - usuwanie\n", clients[i].name);
                    clients[i].active = 0;
                    memset(&clients[i], 0, sizeof(Client));
                    client_count--;
                } else {
                    // Wyślij ping
                    sendto(server_socket, alive_msg, strlen(alive_msg), 0,
                           (struct sockaddr*)&clients[i].addr, sizeof(struct sockaddr_in));
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Użycie: %s <port>\n", argv[0]);
        return 1;
    }
    
    int port = atoi(argv[1]);
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    // Obsługa sygnałów
    signal(SIGINT, cleanup_and_exit);
    
    // Tworzenie socketu
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Błąd tworzenia socketu");
        return 1;
    }
    
    // Konfiguracja adresu serwera
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Bindowanie socketu
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Błąd bindowania");
        close(server_socket);
        return 1;
    }
    
    printf("Serwer czatu uruchomiony na porcie %d\n", port);
    
    // Uruchomienie wątku sprawdzającego alive
    pthread_t alive_thread;
    pthread_create(&alive_thread, NULL, alive_checker, NULL);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE - 1, 0,
                                     (struct sockaddr*)&client_addr, &addr_len);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            // Parsowanie komendy
            char* command = strtok(buffer, " ");
            if (command == NULL) continue;
            
            if (strcmp(command, "JOIN") == 0) {
                char* name = strtok(NULL, " ");
                if (name != NULL) {
                    int index = add_client(name, &client_addr);
                    if (index != -1) {
                        printf("Klient %s dołączył do czatu\n", name);
                        send_message(&client_addr, "Dołączono do czatu");
                    } else {
                        send_message(&client_addr, "Błąd: Czat jest pełny");
                    }
                }
            }
            else if (strcmp(command, "LIST") == 0) {
                send_client_list(&client_addr);
            }
            else if (strcmp(command, "2ALL") == 0) {
                int sender_index = find_client_by_addr(&client_addr);
                if (sender_index != -1) {
                    char* message = strtok(NULL, "");
                    if (message != NULL) {
                        broadcast_message(clients[sender_index].name, message, sender_index);
                    }
                }
            }
            else if (strcmp(command, "2ONE") == 0) {
                int sender_index = find_client_by_addr(&client_addr);
                if (sender_index != -1) {
                    char* recipient = strtok(NULL, " ");
                    char* message = strtok(NULL, "");
                    if (recipient != NULL && message != NULL) {
                        send_private_message(clients[sender_index].name, recipient, message);
                    }
                }
            }
            else if (strcmp(command, "STOP") == 0) {
                int client_index = find_client_by_addr(&client_addr);
                if (client_index != -1) {
                    remove_client(client_index);
                }
            }
            else if (strcmp(command, "ALIVE_RESP") == 0) {
                int client_index = find_client_by_addr(&client_addr);
                if (client_index != -1) {
                    pthread_mutex_lock(&clients_mutex);
                    clients[client_index].last_alive = time(NULL);
                    pthread_mutex_unlock(&clients_mutex);
                }
            }
        }
    }
    
    close(server_socket);
    return 0;
}