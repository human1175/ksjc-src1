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

    // QR 코드 인식 및 서버와 통신
    recognize_qr_code(sock);

    close(sock);
    return 0;
}
