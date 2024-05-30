#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

// Global variables for client
int client_socket;
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

void start_client(const char *server_ip, int server_port) {
    struct sockaddr_in server_addr;

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(client_socket);
        return;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        close(client_socket);
        return;
    }

    printf("Connected to the server at %s:%d\n", server_ip, server_port);

    // Create a thread to handle server communication
    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_data_from_server, NULL);

    // Communication loop
    char buffer[1024];
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
}

void* receive_data_from_server(void* arg) {
    DGIST dgist;
    int n;

    while ((n = recv(client_socket, &dgist, sizeof(DGIST), 0)) > 0) {
        printf("Received data from server\n");
        print_dgist(&dgist);
    }

    if (n == 0) {
        printf("Server closed connection\n");
    } else if (n < 0) {
        perror("recv");
    }

    close(client_socket);
    return NULL;
}

void send_qr_result_to_server(const char* qr_result) {
    ClientAction action;
    sscanf(qr_result, "%1d%1d", &action.row, &action.col);
    action.action = move;

    pthread_mutex_lock(&socket_mutex);
    send(client_socket, &action, sizeof(ClientAction), 0);
    pthread_mutex_unlock(&socket_mutex);
}

void print_dgist(DGIST* dgist) {
    printf("========== Players Information ==========\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        printf("Player %d: Row: %d, Col: %d, Score: %d, Bombs: %d\n",
            i + 1,
            dgist->players[i].row,
            dgist->players[i].col,
            dgist->players[i].score,
            dgist->players[i].bomb);
    }

    printf("========== Map Information ==========\n");
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            Node* node = &dgist->map[i][j];
            printf("Node (%d, %d): Status: %d, Score: %d\n",
                node->row,
                node->col,
                node->item.status,
                node->item.score);
        }
    }
}
