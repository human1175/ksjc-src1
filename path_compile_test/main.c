#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "server.h"

#define SERVER_IP "127.0.0.1" // 고정된 서버 IP 주소

// 서버로부터 받은 client_info를 출력하는 함수
void print_client_info(client_info* client) {
    printf("Client Info - Socket: %d, Address: %s, Port: %d, Location: (%d, %d), Score: %d, Bombs: %d\n",
        client->socket, inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port),
        client->row, client->col, client->score, client->bomb);
}

// 서버로부터 받은 Status를 출력하는 함수
void print_status(enum Status status) {
    const char* status_str[] = { "nothing", "item", "trap" };
    printf("Status: %s\n", status_str[status]);
}

// 서버로부터 받은 Item을 출력하는 함수
void print_item(Item* item) {
    print_status(item->status);
    if (item->status == 1) {  // 올바른 비교로 수정
        printf("Score: %d\n", item->score);
    }
}

// 서버로부터 받은 Node를 출력하는 함수
void print_node(Node* node) {
    printf("Node - Location: (%d, %d)\n", node->row, node->col);
    print_item(&(node->item));
}

// 서버로부터 받은 DGIST를 출력하는 함수
void print_dgist(DGIST* dgist) {
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
void print_players(DGIST* dgist) {
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
void print_map(DGIST* dgist) {
    printf("==========PRINT MAP==========\n");
    for (int j = 0; j < MAP_COL; j++) {
        for (int i = 0; i < MAP_ROW; i++) {
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
    ClientAction client_action = { row, col, action };
    send(sock, &client_action, sizeof(ClientAction), 0);
    printf("Client action sent: (%d, %d) Action: %d\n", row, col, action);
}


//------------------------------ 여기서 부터 Path planning & trap code -------------------------------

void set_players(DGIST* dgist, int digit1, int digit2) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_info client = dgist->players[i];
        if (i==0) { 
            player = client; // Set player
        } else {
            opponent = client; // Set opponent
        }
    }
}

//현재 상황을 계산하기 위해 graph 정보를 담는 struct를 새로 만들었음.
typedef struct {
    int cost;
    int benefit;
    int prev_row;
    int prev_col;
} NodeInfo;

// graph 초기화
int itialize_graph(DGIST* dgist, NodeInfo graph[MAP_ROW][MAP_COL], client_info* opponent) {
    
    double total2b[MAP_ROW][MAP_COL];
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            graph[i][j].cost = 0;
            graph[i][j].benefit = 0;
            graph[i][j].prev_row = -1;
            graph[i][j].prev_col = -1;

            //item이 있을 경우 그 노드에 item score만큼 benefit 부여
            Item item = dgist->map[i][j].item;
            if (item.status == item) {
                graph[i][j].benefit = item.score;
            }
            //trap이 있을 경우 그 노드에 -8만큼 benefit 부여
            else if (item.status == trap) {
                graph[i][j].benefit = -SCORE_DEDUCTION;
            }

            //이웃의 benefit 추가
            int total_benefit = 0;
            if (i - 1 >= 0) total_benefit += graph[i - 1][j].benefit; // North
            if (i + 1 < MAP_ROW) total_benefit += graph[i + 1][j].benefit; // South
            if (j - 1 >= 0) total_benefit += graph[i][j - 1].benefit; // West
            if (j + 1 < MAP_COL) total_benefit += graph[i][j + 1].benefit; // East
            total2b[i][j] = 0.5*total_benefit;
        }
    }

    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            graph[i][j].benefit += total2b[i][j];
        }
    }

    // opponent와 가까운 node에 cost 부여 (내가 임의로 가까울수록 높은 cost 부여하게 만들었음)
    for (int k = 1; k < 3; k++) {
        if (opponent.row + k < MAP_ROW) graph[opponent->row + k][opponent->col].cost = 3 - k;
        if (opponent.col + k < MAP_COL) graph[opponent->row - k][opponent->col].cost = 3 - k;
        if (opponent.row - k >= 0) graph[opponent->row][opponent->col + k].cost = 3 - k;
        if (opponent.col - k >= 0) graph[opponent->row][opponent->col - k].cost = 3 - k;
    }
    if (opponent.row + k < MAP_ROW && opponent.col + k < MAP_COL)graph[opponent->row + 1][opponent->col + 1].cost = 1;
    if (opponent.row - k >= 0 && opponent.col - k >= 0) graph[opponent->row + 1][opponent->col + 1].cost = 1;

    int start_row = player.row;
    int start_col = player.col;
    return start_row, start_col;
}

void calculate_move_cost(int current_row, int current_col, NodeInfo graph[MAP_ROW][MAP_COL]) {
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            graph[i][j].cost += abs(current_row - i) + abs(current_col - j);
        }
    }
}

// 구조체 정의: 최소값과 인덱스를 저장
typedef struct {
    int min_value;
    int min_row;
    int min_col;
} MinValueIndex;

// 2차원 배열에서 최소값과 인덱스를 찾는 함수
MinValueIndex findMinValueAndIndex(int grid[GRID_SIZE][GRID_SIZE]) {
    MinValueIndex result;
    result.min_value = grid[0][0];
    result.min_row = 0;
    result.min_col = 0;

    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            if (grid[row][col] < result.min_value) {
                result.min_value = grid[row][col];
                result.min_row = row;
                result.min_col = col;
            }
        }
    }

    return result;
}

void calculate_benefit(int current_row, int current_col, NodeInfo graph[MAP_ROW][MAP_COL]) {
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            graph[i][j].cost += abs(current_row - i) + abs(current_col - j);
        }
    }
}

// 최종 cost 계산
double calculate_final_cost(int i_cost, int i_benefit) {
    if (benefit > 0) {
        return (double) i_cost / i_benefit;
    }
    else {
        return (double) i_cost * abs(i_benefit);
    }
}


// 최적의 path with Dijkstra's algorithm
void find_optimal_path(DGIST* dgist, NodeInfo graph[MAP_ROW][MAP_COL]) {
    //int start_row = player.row;
    //int start_col = player.col;

    double total[MAP_ROW][MAP_COL];


    // 이전 노드 정보 초기화
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            prev_row[i][j] = -1;
            prev_col[i][j] = -1;
        }
    }

    while (1) {
        // cost 최소인 node 찾기
        for (int i = 0; i < MAP_ROW; i++) {
            for (int j = 0; j < MAP_COL; j++) {
                total[i][j] = calculate_final_cost(graph[i][j].cost, graph[i][j].benefit);
            }
        }

        if total[player.row + 1][player.col + 1] 

            //greedy





// 현재 위치와 목표 위치를 비교하여 이동 방향을 결정하고 이동하는 함수
void Move_To_Node(int i2c_file, int current_row, int current_col, int target_row, int target_col) {
    // 목표 위치와 현재 위치를 비교하여 이동 방향을 결정
    if (current_row < target_row) {
        // 아래쪽으로 이동해야 함
        Car_Run(i2c_file, 70, 70); // 전진
    }
    else if (current_row > target_row) {
        // 위쪽으로 이동해야 함
        // 이동 방향에 따라서 회전 필요
        Car_Spin_Left(i2c_file, 60, 60); // 왼쪽으로 회전
        Car_Run(i2c_file, 70, 70); // 전진
    }
    else if (current_col < target_col) {
        // 오른쪽으로 이동해야 함
        Car_Spin_Right(i2c_file, 60, 60); // 오른쪽으로 회전
        Car_Run(i2c_file, 70, 70); // 전진
    }
    else if (current_col > target_col) {
        // 왼쪽으로 이동해야 함
        Car_Spin_Left(i2c_file, 60, 60); // 왼쪽으로 회전
        Car_Run(i2c_file, 70, 70); // 전진
    }
}


// 최적의 길을 따라가는 함수, 위의 line tracer와의 iterction을 해야 다시 완성할 수 있을듯
void trace_optimal_path(NodeInfo graph[MAP_ROW][MAP_COL], int start_row, int start_col, int end_row, int end_col) {
    int row = end_row;
    int col = end_col;

    while (row != start_row || col != start_col) {
        printf("Path: (%d, %d)\n", row, col);
        int prev_row = graph[row][col].prev_row;
        int prev_col = graph[row][col].prev_col;
        row = prev_row;
        col = prev_col;
    }
    printf("Path: (%d, %d)\n", start_row, start_col);
}

void execute_strategy(DGIST* dgist, int sock) {

    int current_row, current_col = player.row, player.col
    NodeInfo graph[MAP_ROW][MAP_COL];
    initialize_graph(dgist, graph, opponent);
    calculate_move_cost(current_row, current_col, graph[MAP_ROW][MAP_COL]);
    find_optimal_path(dgist, graph);

    // For demonstration, assume we want to move to the bottom-right corner
    int target_row = MAP_ROW - 1;
    int target_col = MAP_COL - 1;
 
    trace_optimal_path(graph, self->row, self->col, target_row, target_col);

    // Example of sending the action to the server
    send_client_action(sock, target_row, target_col, move);
}


//main 함수에서 뭘 받고 부를건지 생각해야되는데, 위에 수정이 끝나야 부를 수 있음.
int main(int argc, char* argv[]) {
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
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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

        if player.row


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