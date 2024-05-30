#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sock = connect_to_server(port);
    if (sock < 0) {
        return 1;
    }

    // 간단한 테스트용 클라이언트 액션 전송
    ClientAction action;
    action.row = 1;
    action.col = 2;
    action.action = move;

    send_client_action(sock, &action);
    receive_server_response(sock);

    close(sock);
    return 0;
}
