#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <wiringPi.h>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"
#include "qr_recognition.h"
// #include "server.h"

// Constants
#define MAX_CLIENTS 2
#define _MAP_ROW 4
#define _MAP_COL 4
#define MAP_ROW _MAP_ROW + 1
#define MAP_COL _MAP_COL + 1
#define MAP_SIZE MAP_COL * MAP_ROW
const int MAX_SCORE = 4; // Item max score
const int SETTING_PERIOD = 20; //Broadcast & Item generation period
const int INITIAL_ITEM = 10; //Initial number of item
const int INITIAL_BOMB = 4; //The number of bomb for each user
const int SCORE_DEDUCTION = 2; //The amount of score deduction due to bomb

// Enums and Structs
enum Status { nothing, item, trap };
typedef struct { enum Status status; int score; } Item;
typedef struct { int row, col; Item item; } Node;
typedef struct { int socket; struct sockaddr_in address; int row, col, score, bomb; } client_info;
typedef struct { client_info players[MAX_CLIENTS]; Node map[MAP_ROW][MAP_COL]; } DGIST;
enum Action { move, setBomb };
typedef struct { int row, col; enum Action action; } ClientAction;

// Global variables
int i2c_file;  // Global variable to store the I2C file descriptor
struct timeval start_time;  // Start time of the program
int server_socket;  // Socket for server communication
pthread_t receive_thread;  // Thread for receiving server data

// Function declarations
void* receive_data_from_server(void* arg);
void print_dgist(DGIST* dgist);
void handle_sigint(int sig);
long get_elapsed_time(struct timeval start);
void visualize_sensor_values(int left1, int left2, int right1, int right2, char* buffer);
void line_tracer();

// Signal handler to stop the motors and clean up
void handle_sigint(int sig) {
    Car_Stop(i2c_file);
    close(i2c_file);
    close(server_socket);
    printf("\n\n================================================================================\n               Motors stopped, I2C file closed, and server disconnected. Exit Complete!\n================================================================================\n\n");
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

// Function to visualize sensor values
void visualize_sensor_values(int left1, int left2, int right1, int right2, char* buffer) {
    buffer[0] = left1 == LOW ? '|' : ' ';
    buffer[1] = left2 == LOW ? '|' : ' ';
    buffer[2] = right1 == LOW ? '|' : ' ';
    buffer[3] = right2 == LOW ? '|' : ' ';
    buffer[4] = '\0';
}

// Function to read sensors and control the car accordingly
void line_tracer() {
    while (1) {
        int left1_value = digitalRead(LEFT1_PIN);
        int left2_value = digitalRead(LEFT2_PIN);
        int right1_value = digitalRead(RIGHT1_PIN);
        int right2_value = digitalRead(RIGHT2_PIN);

        long elapsed_time = get_elapsed_time(start_time);
        char sensor_visual[5];
        visualize_sensor_values(left1_value, left2_value, right1_value, right2_value, sensor_visual);

        // Print sensor values for debugging
        printf("[%ld ms] Sensors: %s\n", elapsed_time, sensor_visual);

        // Implement the logic based on sensor values
        if ((left1_value == LOW || left2_value == LOW) && right2_value == LOW) {
            printf("[%ld ms] Turning right (sharp)\n", elapsed_time);
            Car_Spin_Right(i2c_file, 70, 30);
            usleep(200000);  // 0.2 seconds

        } else if (left1_value == LOW && (right1_value == LOW || right2_value == LOW)) {
            printf("[%ld ms] Turning left (sharp)\n", elapsed_time);
            Car_Spin_Left(i2c_file, 30, 70);
            usleep(200000);  // 0.2 seconds

        } else if (left1_value == LOW) {
            printf("[%ld ms] Turning left\n", elapsed_time);
            Car_Spin_Left(i2c_file, 70, 70);
            usleep(50000);  // 0.05 seconds

        } else if (right2_value == LOW) {
            printf("[%ld ms] Turning right\n", elapsed_time);
            Car_Spin_Right(i2c_file, 70, 70);
            usleep(50000);  // 0.05 seconds

        } else if (left2_value == LOW && right1_value == HIGH) {
            printf("[%ld ms] Adjusting left\n", elapsed_time);
            Car_Spin_Left(i2c_file, 60, 60);
            usleep(20000);  // 0.02 seconds

        } else if (left2_value == HIGH && right1_value == LOW) {
            printf("[%ld ms] Adjusting right\n", elapsed_time);
            Car_Spin_Right(i2c_file, 60, 60);
            usleep(20000);  // 0.02 seconds

        } else if (left2_value == LOW && right1_value == LOW) {
            printf("[%ld ms] Moving straight\n", elapsed_time);
            Car_Run(i2c_file, 70, 70);
        }

        usleep(10000);  // 10 milliseconds delay to prevent excessive CPU usage
    }
}

// Thread function to receive data from server
void* receive_data_from_server(void* arg) {
    DGIST dgist;
    while (1) {
        if (recv(server_socket, &dgist, sizeof(DGIST), 0) > 0) {
            print_dgist(&dgist);
        }
    }
    return NULL;
}

// Print DGIST structure information
void print_dgist(DGIST* dgist) {
    printf("\n========== DGIST Information ==========\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        printf("Player %d: Row: %d, Col: %d, Score: %d, Bomb: %d\n",
               i + 1, dgist->players[i].row, dgist->players[i].col, dgist->players[i].score, dgist->players[i].bomb);
    }
    printf("========== Map Information ==========\n");
    for (int i = 0; i < MAP_ROW; i++) {
        for (int j = 0; j < MAP_COL; j++) {
            printf("(%d, %d): Status: %d, Score: %d\n",
                   dgist->map[i][j].row, dgist->map[i][j].col, dgist->map[i][j].item.status, dgist->map[i][j].item.score);
        }
    }
    printf("======================================\n");
}

int main(int argc, char *argv[]) {
    const char *filename = "/dev/i2c-1";
    i2c_file = open_i2c_device(filename);

    if (i2c_file < 0) {
        return -1;
    }

    setup_gpio();

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    struct sockaddr_in server_addr;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return 1;
    }

    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to the server failed");
        return 1;
    }

    printf("Connected to the server\n");

    pthread_create(&receive_thread, NULL, receive_data_from_server, NULL);

    // Set up the SIGINT signal handler
    signal(SIGINT, handle_sigint);

    // Record the start time
    gettimeofday(&start_time, NULL);

    // Start QR code recognition in a separate thread
    recognize_qr_code_thread();

    // Start line tracing
    line_tracer();

    // This point will never be reached due to the infinite loop in line_tracer
    close(i2c_file);
    close(server_socket);
    return 0;
}
