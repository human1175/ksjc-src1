#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int connect_to_server() {
    int sock;
    struct sockaddr_in server_addr;

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to the server...\n");
    return sock;
}

void send_client_action(int sock, ClientAction *action) {
    if (send(sock, action, sizeof(ClientAction), 0) < 0) {
        perror("Send failed");
    } else {
        printf("Sent action: row=%d, col=%d, action=%d\n", action->row, action->col, action->action);
    }
}

void receive_server_response(int sock) {
    DGIST dgist;
    int len = recv(sock, &dgist, sizeof(DGIST), 0);
    if (len > 0) {
        printf("Received map and player information from server.\n");
        // 서버로부터 받은 정보를 출력하거나 처리하는 로직 추가
    } else {
        perror("Receive failed");
    }
}

int main() {
    int sock = connect_to_server();
    if (sock < 0) {
        return 1;
    }

    // 예시 클라이언트 액션
    ClientAction action;
    action.row = 1;
    action.col = 2;
    action.action = move;

    send_client_action(sock, &action);
    receive_server_response(sock);

    close(sock);
    return 0;
}
