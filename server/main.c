#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"

// 외부 함수 선언
int connect_to_server(int port);
void recognize_qr_code(int sock);
void send_client_action(int sock, ClientAction *action);
void receive_server_response(int sock);

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

    // QR 코드 인식 및 서버와 통신
    recognize_qr_code(sock);

    // 여러 메시지를 보내는 테스트
    ClientAction actions[3] = {
        {1, 2, move},
        {2, 3, setBomb},
        {3, 4, move}
    };

    for (int i = 0; i < 3; i++) {
        send_client_action(sock, &actions[i]);
        receive_server_response(sock);
        sleep(1); // 잠시 대기
    }

    close(sock);
    return 0;
}
