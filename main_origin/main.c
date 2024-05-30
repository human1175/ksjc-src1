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

#define SERVER_IP "127.0.0.1" // 고정된 서버 IP 주소

int i2c_file;  // Global variable to store the I2C file descriptor
struct timeval start_time;  // Start time of the program
int sock;  // 소켓 파일 디스크립터

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

// Function to read sensors and control the car accordingly
void line_tracer() {
    while (1) {
        int left1_value = digitalRead(LEFT1_PIN);
        int left2_value = digitalRead(LEFT2_PIN);
        int right1_value = digitalRead(RIGHT1_PIN);
        int right2_value = digitalRead(RIGHT2_PIN);

        long elapsed_time = get_elapsed_time(start_time);

        if (left1_value == LOW && right2_value == LOW) {
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
            Car_Run(i2c_file, 120, 120);
        }

        usleep(10000);  // 10 milliseconds delay to prevent excessive CPU usage
    }
}

// QR 코드 인식을 수행하고 결과를 서버로 전송하는 함수
void* qr_code_thread_func(void* arg) {
    recognize_qr_code_thread(); // QR 코드 인식 함수 호출
    return NULL;
}

// QR 코드 인식 콜백 함수
extern "C" void qr_code_callback(const char* qr_code_data) {
    printf("QR Code Recognized: %s\n", qr_code_data);

    // 서버로 QR 코드 데이터를 전송합니다.
    send(sock, qr_code_data, strlen(qr_code_data), 0);
    printf("QR code data sent to the server.\n");
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

    // QR 코드 인식과 전송을 별도의 스레드에서 시작
    pthread_t qr_thread;
    pthread_create(&qr_thread, NULL, qr_code_thread_func, NULL);

    // 라인 트레이서 기능 시작
    line_tracer();

    // 이 지점은 line_tracer의 무한 루프 때문에 도달하지 않음
    pthread_join(qr_thread, NULL);
    close(i2c_file);
    close(sock);
    return 0;
}
