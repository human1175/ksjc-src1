#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "qr_recognition.h"

// Global variables for client
int client_socket;
pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to start the client and connect to the server
void start_client(const char *server_ip, int server_port) {
    struct sockaddr_in server_addr;
    char buffer[1024];
    int n;

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
    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(client_socket);
}

void* receive_data_from_server(void* arg) {
    char buffer[1024];
    int n;

    while ((n = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("Server: %s\n", buffer);
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
    pthread_mutex_lock(&socket_mutex);
    send(client_socket, qr_result, strlen(qr_result), 0);
    pthread_mutex_unlock(&socket_mutex);
}
