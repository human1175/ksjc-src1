#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

#define SERVER_IP "127.0.0.1" // 고정된 서버 IP 주소

// 서버로부터 받은 client_info를 출력하는 함수
void print_client_info(client_info *client) {
    printf("Client Info - Socket: %d, Address: %s, Port: %d, Location: (%d, %d), Score: %d, Bombs: %d\n",
           client->socket, inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port), 
           client->row, client->col, client->score, client->bomb);
}

// 서버로부터 받은 Status를 출력하는 함수
void print_status(enum Status status) {
    const char *status_str[] = {"nothing", "item", "trap"};
    printf("Status: %s\n", status_str[status]);
}

// 서버로부터 받은 Item을 출력하는 함수
void print_item(Item *item) {
    print_status(item->status);
    if (item->status == 1) {  // 올바른 비교로 수정
        printf("Score: %d\n", item->score);
    }
}

// 서버로부터 받은 Node를 출력하는 함수
void print_node(Node *node) {
    printf("Node - Location: (%d, %d)\n", node->row, node->col);
    print_item(&(node->item));
}

// 서버로부터 받은 DGIST를 출력하는 함수
void print_dgist(DGIST *dgist) {
    printf("DGIST - Players Info:\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        print_client_info(&(dgist->players[i]));
    }
    printf("DGIST - Map Info:\n");
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            print_node(&(dgist->map[i][j]));
        }
    }
}

// 서버로부터 받은 DGIST 구조체의 players 정보를 출력하는 함수
void print_players(DGIST *dgist) {
    printf("==========PRINT PLAYERS==========\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_info client = dgist->players[i];
        printf("++++++++++Player %d++++++++++\n", i + 1);
        printf("Location: (%d, %d)\n", client.row, client.col);
        printf("Score: %d\n", client.score);
        printf("Bomb: %d\n", client.bomb);
    }
    printf("==========PRINT DONE==========\n");
}

// 맵 정보를 출력하는 함수
void print_map(DGIST *dgist) {
    printf("==========PRINT MAP==========\n");
    for (int i = MAP_ROW - 1; i >= 0; i--) {
        for (int j = 0; j < MAP_COL; j++) {
            Item tmpItem = (dgist->map[i][j]).item;
            switch (tmpItem.status) {
                case nothing:
                    printf("- ");
                    break;
                case item:
                    printf("%d ", tmpItem.score);
                    break;
                case trap:
                    printf("x ");
                    break;
            }
        }
        printf("\n");
    }
    printf("==========PRINT DONE==========\n");
}

void send_client_action(int sock, int row, int col, int action) {
    // 클라이언트의 위치와 행동을 서버로 전송하는 함수
    ClientAction client_action = {row, col, action};
    send(sock, &client_action, sizeof(ClientAction), 0);
    printf("Client action sent: (%d, %d) Action: %d\n", row, col, action);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sock;
    struct sockaddr_in server_addr;
    DGIST dgist;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // 서버 IP 주소 설정
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return -1;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server.\n");

    // 서버로부터 데이터 수신
    while (1) {
        int valread = read(sock, &dgist, sizeof(DGIST));
        if (valread <= 0) {
            perror("Read error");
            break;
        }

        // 서버로부터 받은 데이터 처리
        // print_dgist(&dgist);
        print_players(&dgist); // 추가된 함수 호출
        print_map(&dgist); // 추가된 함수 호출

        // 클라이언트 액션 설정 및 서버로 전송
        int row = 1; // 예시로 설정한 행 위치
        int col = 2; // 예시로 설정한 열 위치
        int action = move; // 예시로 설정한 액션

        send_client_action(sock, row, col, action);

        // 주기적으로 업데이트를 받기 위해 잠시 대기
        sleep(1);
    }

    close(sock);
    return 0;
}
