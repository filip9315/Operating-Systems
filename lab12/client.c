#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>

#define MAX_NAME_LEN 32
#define MAX_MSG_LEN 512
#define BUFFER_SIZE 1024

int client_socket;
struct sockaddr_in server_addr;
char client_name[MAX_NAME_LEN];
volatile int running = 1;

void cleanup_and_exit(int sig) {
    printf("\nWylogowywanie z czatu...\n");
    
    // Wyślij komendę STOP do serwera
    char stop_msg[] = "STOP";
    sendto(client_socket, stop_msg, strlen(stop_msg), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    running = 0;
    close(client_socket);
    exit(0);
}

void* message_receiver(void* arg) {
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);
    
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        
        int bytes_received = recvfrom(client_socket, buffer, BUFFER_SIZE - 1, 0,
                                     (struct sockaddr*)&server_addr, &addr_len);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            // Sprawdź czy to ping od serwera
            if (strcmp(buffer, "ALIVE") == 0) {
                char alive_resp[] = "ALIVE_RESP";
                sendto(client_socket, alive_resp, strlen(alive_resp), 0,
                       (struct sockaddr*)&server_addr, sizeof(server_addr));
            } else {
                printf("%s\n", buffer);
                printf("> "); // Wyświetl prompt ponownie
                fflush(stdout);
            }
        }
    }
    return NULL;
}

void send_message(const char* message) {
    sendto(client_socket, message, strlen(message), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Użycie: %s <nazwa_klienta> <adres_serwera> <port>\n", argv[0]);
        printf("Przykład: %s Jan 127.0.0.1 8080\n", argv[0]);
        return 1;
    }
    
    // Sprawdź długość nazwy klienta
    if (strlen(argv[1]) >= MAX_NAME_LEN) {
        printf("Błąd: Nazwa klienta zbyt długa (max %d znaków)\n", MAX_NAME_LEN - 1);
        return 1;
    }
    
    strcpy(client_name, argv[1]);
    char* server_ip = argv[2];
    int server_port = atoi(argv[3]);
    
    // Obsługa sygnałów
    signal(SIGINT, cleanup_and_exit);
    
    // Tworzenie socketu
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Błąd tworzenia socketu");
        return 1;
    }
    
    // Konfiguracja adresu serwera
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Błąd: Nieprawidłowy adres IP\n");
        close(client_socket);
        return 1;
    }
    
    // Dołącz do czatu
    char join_msg[BUFFER_SIZE];
    snprintf(join_msg, sizeof(join_msg), "JOIN %s", client_name);
    send_message(join_msg);
    
    // Uruchomienie wątku odbierającego wiadomości
    pthread_t receiver_thread;
    pthread_create(&receiver_thread, NULL, message_receiver, NULL);
    
    printf("Połączono z serwerem czatu jako '%s'\n", client_name);
    
    char input[BUFFER_SIZE];
    while (running) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Usuń znak nowej linii
        input[strcspn(input, "\n")] = 0;
        
        if (strlen(input) == 0) {
            continue;
        }
        
        // Parsowanie komend
        char* command = strtok(input, " ");
        
        if (strcmp(command, "STOP") == 0) {
            break;
        }
        else if (strcmp(command, "LIST") == 0) {
            send_message("LIST");
        }
        else if (strcmp(command, "2ALL") == 0) {
            char* message = strtok(NULL, "");
            if (message != NULL) {
                char full_msg[BUFFER_SIZE];
                snprintf(full_msg, sizeof(full_msg), "2ALL %s", message);
                send_message(full_msg);
            } else {
                printf("Błąd: Brak wiadomości do wysłania\n");
            }
        }
        else if (strcmp(command, "2ONE") == 0) {
            char* recipient = strtok(NULL, " ");
            char* message = strtok(NULL, "");
            if (recipient != NULL && message != NULL) {
                char full_msg[BUFFER_SIZE];
                snprintf(full_msg, sizeof(full_msg), "2ONE %s %s", recipient, message);
                send_message(full_msg);
            } else {
                printf("Błąd: Nieprawidłowa składnia. Użyj: 2ONE <nazwa_klienta> <wiadomość>\n");
            }
        }
        else {
            printf("Nieznana komenda. Wpisz 'HELP' aby zobaczyć dostępne komendy.\n");
        }
    }
    
    cleanup_and_exit(0);
    return 0;
}