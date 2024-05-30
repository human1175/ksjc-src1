#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "server.h"
#include "client.h"
#include "qr_recognition.h"

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

    // QR 코드 인식 스레드 시작
    start_qr_recognition(sock);

    // 메인 스레드는 서버와 통신을 지속합니다.
    while (1) {
        receive_server_response(sock);
        sleep(1); // 주기적으로 서버 응답 확인
    }

    // 종료 시 QR 코드 인식 스레드 중지
    stop_qr_recognition();

    close(sock);
    return 0;
}
