#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"
#include "qr_recognition.h"
#include "server.h"

// #define SERVER_IP "10.150.4.149" // 고정된 서버 IP 주소
// #define SERVER_IP "127.0.0.1"
#define SERVER_IP ""

int i2c_file;  // Global variable to store the I2C file descriptor
struct timeval start_time;  // Start time of the program
int sock;  // 소켓 파일 디스크립터
int pre_row_nmy = -1; //ssy
int pre_col_nmy = -1; //ssy
int pre_row = -1; //ssy
int pre_col = -1; //ssy
client_info player; // ssy
client_info oppoent; // ssy

int bomb_row = -1;  // 전역 변수로 폭탄을 설치할 행을 저장
int bomb_col = -1;  // 전역 변수로 폭탄을 설치할 열을 저장
pthread_mutex_t bomb_mutex;  // mutex for synchronizing access to bomb_row and bomb_col


// Signal handler to stop the motors and clean up
void handle_sigint(int sig) {
    Car_Stop(i2c_file);
    close(i2c_file);
    close(sock);
    printf("\n\n================================================================================\n               Motors stopped and I2C file closed. Exit Complete!\n================================================================================\n\n");
    exit(0);
}

// Function to get elapsed time in milliseconds
long get_elapsed_time(struct timeval start) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long seconds = current_time.tv_sec - start.tv_sec;
    long microseconds = current_time.tv_usec - start.tv_usec;
    return (seconds * 1000) + (microseconds / 1000);
}

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
   for (int i = 0; i < MAP_ROW; i++) {
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

// 서버로부터 실시간으로 값을 받아와서 출력하는 함수
void* receive_from_server(void* arg) {
    int server_sock = *(int*)arg;
    DGIST dgist;

    while (1) {
        int valread = read(server_sock, &dgist, sizeof(DGIST));
        if (valread <= 0) {
            perror("Read error");
            break;
        }

        // 서버로부터 받은 데이터 처리
        print_players(&dgist);
        print_map(&dgist);
    }
    return NULL;
}

//------------------------------ 여기서 부터 Path planning & trap
client_info player; // 전역변수
client_info oppoent; // 전역변수
int pre_row = -1; //ssy
int pre_col = -1; //ssy
void set_players(DGIST* dgist) {
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
int find_optimal_path(DGIST* dgist, NodeInfo graph[MAP_ROW][MAP_COL]) {
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

    // cost 최소인 node 찾기
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            total[i][j] = calculate_final_cost(graph[i][j].cost, graph[i][j].benefit);
        }
    }
    
    int destination_node_row = findMinValueAndIndex(total).min_row;
    int destination_node_col = findMinValueAndIndex(total).min_col;
    
    set_players(&dgist)
    int cur_row = -1;
    int cur_col = -1;

    int svr_row = player.row;
    int svr_col = player.col;
    
    int drc = 0; //1,2,3,4 각각 북동남서(시계방향)
    //현재 이전 위치를 저장하고 현재 위치를 서버에서 업데이트한다.
    if (cur_row != svr_row) { //cycle이 돌 때마다 업데이트하면 pre와 cur이 거의 동일하게 갈 거기 때문에 변화한 경우에만 업데이트한다.
        pre_row = cur_row;
        cur_row = svr_row;
    }
    if (cur_col != svr_col) {
        pre_col = cur_col;
        cur_col = svr_col;
    }

    //현재 방향을 구한다.
    if (cur_row - pre_row > 0) {//북
        drc = 3;
    }
    if (cur_row - pre_row < 0) {//동
        drc = 1;
    }
    if (cur_col - pre_col > 0) {//남
        drc = 2;
    }
    if (cur_col - pre_col < 0) {//서
        drc = 4;
    }
    
    int row_diff = destination_node_row - player.row;
    int col_diff = destination_node_col - player.col;
    int directions[2]; //내가 움직일 방향(최종적으로는 [1,0], [0,-1] 뭐 이런식으로 나올거임.)
    if (row_diff > 0) {
        directions[0] = 1;
    } else if (row_diff < 0) {
        directions[0] = -1;
    } else {
        directions[0] = 0;
    }

    if (col_diff > 0) {
        directions[1] = 1;
    } else if (col_diff < 0) {
        directions[1] = -1;
    } else {
        directions[1] = 0;
    }

    if (directions[0]!=0 && directions[1]!=0){
        if (total[player.row+directions[0]][player.col] < total[player.row][player.col+directions[1]]){
            directions[1]=0;
        }
        else{
            directions[0]=0;
        }
    }

    int drc2; //1,2,3,4 각각 북동남서(시계방향)
    

    for (int i = 0; i < 2; i++){
        if (directions[i] != 0){
            if (i == 0){
                if (directions[0] > 0){
                    drc2 = 1;
                }
                else {
                    drc2 = 3;
                }
            }
            else if (i == 1){
                if (directions[1] > 0){
                    drc2 = 2;
                }
                else {
                    drc2 = 4;
                }
            }
        }
    }
    
    
    int handle; //좌회전0 직진1 우회전2
    if (drc==drc2){
        handle = 1;
    }
    else if ((drc==1 && drc2==2) || (drc==2 && drc2==3) || (drc==3 && drc2==4) || (drc==4 && drc2==1)){
        handle = 2;
    }
    else if ((drc==1 && drc2==4) || (drc==2 && drc2==1) || (drc==3 && drc2==2) || (drc==4 && drc2==3)){
        handle = 0;
    }
    else{
        handle = 2;
    }

    return handle;
}





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

int execute_strategy(DGIST* dgist, int sock) {

    int current_row, current_col = player.row, player.col
    NodeInfo graph[MAP_ROW][MAP_COL];
    initialize_graph(dgist, graph, opponent);
    calculate_move_cost(current_row, current_col, graph[MAP_ROW][MAP_COL]);
    return find_optimal_path(dgist, graph);
    /*
    // For demonstration, assume we want to move to the bottom-right corner
    int target_row = MAP_ROW - 1;
    int target_col = MAP_COL - 1;
 
    trace_optimal_path(graph, self->row, self->col, target_row, target_col);

    // Example of sending the action to the server
    send_client_action(sock, target_row, target_col, move);
    */
}
//여기까지
int leave_trap(int row_dst, int col_dst) {
    int cur_row_nmy = -1;
    int cur_col_nmy = -1;

    int svr_row_nmy = opponent.row;
    int svr_col_nmy = opponent.col;

    int trap = 0;
    int drc = 0; //1,2,3,4 각각 북동남서(시계방향)
    //상대방의 이전 위치를 저장하고 현재 위치를 서버에서 업데이트한다.
    if (cur_row_nmy != svr_row_nmy) { //cycle이 돌 때마다 업데이트하면 pre와 cur이 거의 동일하게 갈 거기 때문에 변화한 경우에만 업데이트한다.
        pre_row_nmy = cur_row_nmy;
        cur_row_nmy = svr_row_nmy;
    }
    if (cur_col_nmy != svr_col_nmy) {
        pre_col_nmy = cur_col_nmy;
        cur_col_nmy = svr_col_nmy;
    }

    //상대방의 방향을 구한다.
    if (cur_row_nmy - pre_row_nmy > 0) {//북
        drc = 3;
    }
    if (cur_row_nmy - pre_row_nmy < 0) {//동
        drc = 1;
    }
    if (cur_col_nmy - pre_col_nmy > 0) {//남
        drc = 2;
    }
    if (cur_col_nmy - pre_col_nmy < 0) {//서
        drc = 4;
    }

    //case1
    if (drc == 1 || drc == 4) {
        if (cur_col_nmy == 2 && col_dst == 2) {
            if (cur_row_nmy == 0 && row_dst == 1) {
                trap = 1;
            }
            else if (cur_row_nmy == 4 && row_dst == 3) {
                trap = 1;
            }
        }
    }
    else if (drc == 2 || drc == 3) {
        if (cur_row_nmy == 2 && row_dst == 2) {
            if (cur_col_nmy == 4 && col_dst == 3) {
                trap = 1;
            }
            else if (cur_col_nmy == 0 && col_dst == 1) {
                trap = 1;
            }
        }
    }

    //case2
    if (drc == 1 || drc == 4) {
        if (cur_col_nmy == 3 && col_dst == 3) {
            if (cur_row_nmy == 0 && row_dst == 1) {
                trap = 1;
            }
            if (cur_row_nmy == 4 && row_dst == 3) {
                trap = 1;
            }
        }
        else if (cur_col_nmy == 1 && col_dst == 1) {
            if (cur_row_nmy == 4 && row_dst == 3) {
                trap = 1;
            }
            if (cur_row_nmy == 0 && row_dst == 1) {
                trap = 1;
            }
        }
    }
    else if (drc == 2 || drc == 3) {
        if (cur_col_nmy == 4 && col_dst == 3) {
            if (cur_row_nmy == 1 && row_dst == 1) {
                trap = 1;
            }
            if (cur_row_nmy == 3 && row_dst == 3) {
                trap = 1;
            }
        }
        else if (cur_col_nmy == 0 && col_dst == 1) {
            if (cur_row_nmy == 3 && row_dst == 3) {
                trap = 1;
            }
            if (cur_row_nmy == 1 && row_dst == 1) {
                trap = 1;
            }
        }
    }

    //case3
    if (cur_col_nmy == 4 && col_dst == 3) {
        if (cur_row_nmy == 0 && row_dst == 1) {
            trap = 1;
        }
        else if (cur_row_nmy == 4 && row_dst == 3) {
            trap = 1;
        }
    }
    else if (cur_col_nmy == 0 && col_dst == 1) {
        if (cur_row_nmy == 0 && row_dst == 1) {
            trap = 1;
        }
        else if (cur_row_nmy == 4 && row_dst == 3) {
            trap = 1;
        }
    }

    //case4
    if (drc == 1) {
        if (cur_row_nmy == 0 && row_dst == 0) {
            if (cur_col_nmy == 4 && col_dst == 3) {
                trap = 1;
            }
            else if (cur_col_nmy == 0 && col_dst == 1) {
                trap = 1;
            }
        }
    }
    else if (drc == 2) {
        if (cur_col_nmy == 4 && col_dst == 4) {
            if (cur_row_nmy == 0 && row_dst == 1) {
                trap = 1;
            }
            else if (cur_row_nmy == 4 && row_dst == 3) {
                trap = 1;
            }
        }
    }
    else if (drc == 3) {
        if (cur_row_nmy == 4 && row_dst == 4) {
            if (cur_col_nmy == 4 && col_dst == 3) {
                trap = 1;
            }
            else if (cur_col_nmy == 0 && col_dst == 1) {
                trap = 1;
            }
        }
    }
    else if (drc == 4) {
        if (cur_col_nmy == 0 && col_dst == 0) {
            if (cur_row_nmy == 4 && row_dst == 3) {
                trap = 1;
            }
            else if (cur_row_nmy == 0 && row_dst == 1) {
                trap = 1;
            }
        }
    }

    //폭탄(지뢰) 설치
    if (trap) {
        return 1;
    }
    else {
        return 0;
    }
}



// QR 코드 인식 콜백 함수
// void qr_code_callback(const char* qr_code_data) {
//     printf("QR Code Recognized: %s\n", qr_code_data);

//     // QR 코드 데이터를 두 자리 숫자로 받아서 한 자리씩 나눕니다.
//     if (strlen(qr_code_data) == 2) {
//         // 각 자릿수를 정수로 변환합니다.
//         int digit1 = qr_code_data[0] - '0';
//         int digit2 = qr_code_data[1] - '0';

//         // 서버로 각 자릿수를 정수로 전송합니다.
//         send(sock, &digit1, sizeof(int), 0);
//         send(sock, &digit2, sizeof(int), 0);
        
//         printf("QR code data sent to the server: %d, %d\n", digit1, digit2);
//     } else {
//         printf("Invalid QR code data length: %s\n", qr_code_data);
//     }
// }

// 클라이언트의 위치와 행동을 서버로 전송하는 함수
void send_client_action(int sock, int row, int col, Action action) {
    ClientAction client_action = {row, col, action};
    send(sock, &client_action, sizeof(ClientAction), 0);
    printf("Client action sent: (%d, %d) Action: %d\n", row, col, action);
}

// QR 코드 인식 콜백 함수
void qr_code_callback(const char* qr_code_data) {
    printf("QR Code Recognized: %s\n", qr_code_data);

    // QR 코드 데이터를 두 자리 숫자로 받아서 한 자리씩 나눕니다.
    if (strlen(qr_code_data) == 2) {
        // 각 자릿수를 정수로 변환합니다.
        int row = qr_code_data[0] - '0';  // 첫 번째 자릿수를 행으로 사용
        int col = qr_code_data[1] - '0';  // 두 번째 자릿수를 열로 사용
        Action action = leave_trap(row, col);  // 폭탄 설치

        // mutex를 사용하여 전역 변수 접근
        pthread_mutex_lock(&bomb_mutex);
        if (row == bomb_row && col == bomb_col) {
            action = setBomb;  // 폭탄 설치
        } else {
            action = move;  // 이동
        }
        pthread_mutex_unlock(&bomb_mutex);

        // 서버로 위치와 지뢰 정보 전송
        send_client_action(sock, row, col, action);
        printf("QR code data sent to the server: Row: %d, Col: %d, Action: %d\n", row, col, action);
    } else {
        printf("Invalid QR code data length: %s\n", qr_code_data);
    }
}

// Function to read sensors and control the car accordingly
void line_tracer() {
    while (1) {
        int left1_value = digitalRead(LEFT1_PIN);
        int left2_value = digitalRead(LEFT2_PIN);
        int right1_value = digitalRead(RIGHT1_PIN);
        int right2_value = digitalRead(RIGHT2_PIN);

        long elapsed_time = get_elapsed_time(start_time);

        bool print_option = 0;

        // 여기에 path planning을 통해 결정된 우회전, 좌회전, 직진 여부를 처리하는 조건문을 넣어야함 // 
        // 조건문의 경우에는 직진은 아래 코드의 직진 부분을, 좌회전은 intersection 감지 및 left결과인 경우 sharp turn을 하게, 우회전도 이와 비슷하게 sharp turn을 하도록 구현하면 됌 // 

        // 폭탄 설치할 노드의 위치가 결정나면 전역변수 bomb_row, bomb_col 아래 주석 코드와 같이 설정해줘야함 // 
        // pthread_mutex_lock(&bomb_mutex);
        // bomb_row = 결정된 row결과;
        // bomb_col = 결정된 col결과;
        // pthread_mutex_unlock(&bomb_mutex);

        if (execute_strategy() == 0) {
            if (left1_value == LOW) {
                if (print_option) {
                    printf("[%ld ms] Turning left (sharp)\n", elapsed_time);
                }
                Car_Spin_Left(i2c_file, 30, 70);
            usleep(200000);  // 0.2 seconds
            }
        }

        else if (execute_strategy() == 2) {
            if (right2_value == LOW) {
                if (print_option) {
                    printf("[%ld ms] Turning right (sharp)\n", elapsed_time);
                }
                Car_Spin_Right(i2c_file, 70, 30);
                usleep(200000);  // 0.2 seconds
            }
        }

        if (left1_value == LOW) {
            if (print_option) {
                printf("[%ld ms] Turning left\n", elapsed_time);
            }
            Car_Spin_Left(i2c_file, 70, 70);
            usleep(50000);  // 0.05 seconds

        }
        else if (right2_value == LOW) {
            if (print_option) {
                printf("[%ld ms] Turning right\n", elapsed_time);
            }
            Car_Spin_Right(i2c_file, 70, 70);
            usleep(50000);  // 0.05 seconds

        }
        else if (left2_value == LOW && right1_value == HIGH) {
            if (print_option) {
                printf("[%ld ms] Adjusting left\n", elapsed_time);
            }
            Car_Spin_Left(i2c_file, 60, 60);
            usleep(20000);  // 0.02 seconds

        }
        else if (left2_value == HIGH && right1_value == LOW) {
            if (print_option) {
                printf("[%ld ms] Adjusting right\n", elapsed_time);
            }
            Car_Spin_Right(i2c_file, 60, 60);
            usleep(20000);  // 0.02 seconds

        }
        else if (left2_value == LOW && right1_value == LOW) {
            if (print_option) {
                printf("[%ld ms] Moving straight\n", elapsed_time);
            }
            Car_Run(i2c_file, 80, 80);
        }

        usleep(10000);  // 10 milliseconds delay to prevent excessive CPU usage
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr;

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

    // I2C 장치 열기
    const char *filename = "/dev/i2c-1";
    i2c_file = open_i2c_device(filename);

    if (i2c_file < 0) {
        return -1;
    }

    setup_gpio();

    // SIGINT 신호 핸들러 설정
    signal(SIGINT, handle_sigint);

    // 시작 시간 기록
    gettimeofday(&start_time, NULL);

    // 서버로부터 값을 받아오는 스레드 생성
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_from_server, &sock);

    // QR 코드 인식과 전송을 별도의 스레드에서 시작
    recognize_qr_code_thread(qr_code_callback);

    // 라인 트레이서 기능 시작
    line_tracer();

    // 이 지점은 line_tracer의 무한 루프 때문에 도달하지 않음
    pthread_join(receive_thread, NULL);
    close(i2c_file);
    close(sock);
    return 0;
}