#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <sys/time.h>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"
#include "qr_recognition.h"

int i2c_file;  // Global variable to store the I2C file descriptor
struct timeval start_time;  // Start time of the program
struct timeval white_start_time;  // Time when all sensors detect white
int white_detected = 0;  // Flag to indicate if white detection has started

// Signal handler to stop the motors and clean up
void handle_sigint(int sig) {
    Car_Stop(i2c_file);
    close(i2c_file);
    printf("\n\nMotors stopped and I2C file closed. Exit Complete!\n============================================================\n");
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
        // printf("[%ld ms] Left1: %d, Left2: %d, Right1: %d, Right2: %d\n", elapsed_time, left1_value, left2_value, right1_value, right2_value);
        // printf("[%ld ms] Sensors: %s\n", elapsed_time, sensor_visual);

        // Check if all sensors are detecting white
        if (left1_value == HIGH && left2_value == HIGH && right1_value == HIGH && right2_value == HIGH) {
            if (!white_detected) {
                // Start the timer if white detection has just started
                gettimeofday(&white_start_time, NULL);
                white_detected = 1;
            } else {
                // Check the elapsed time since white detection started
                long white_elapsed_time = get_elapsed_time(white_start_time);
                if (white_elapsed_time > 1000) {  // 1 second
                    printf("[%ld ms] All sensors detecting white for over 1 second, stopping\n", elapsed_time);
                    Car_Stop(i2c_file);
                    break;
                }
            }
        } else {
            // Reset the white detection flag if any sensor detects the line
            white_detected = 0;

            // Implement the logic based on sensor values
            if ((left1_value == LOW || left2_value == LOW) && right2_value == LOW) {
                printf("[%ld ms] Turning right (sharp)\n", elapsed_time);
                Car_Spin_Right(i2c_file, 70, 30);
                usleep(200000);  // 0.2 seconds

            // Handle sharp and right angles
            } else if (left1_value == LOW && (right1_value == LOW || right2_value == LOW)) {
                printf("[%ld ms] Turning left (sharp)\n", elapsed_time);
                Car_Spin_Left(i2c_file, 30, 70);
                usleep(200000);  // 0.2 seconds

            // Detect most left
            } else if (left1_value == LOW) {
                printf("[%ld ms] Turning left\n", elapsed_time);
                Car_Spin_Left(i2c_file, 70, 70);
                usleep(50000);  // 0.05 seconds

            // Detect most right
            } else if (right2_value == LOW) {
                printf("[%ld ms] Turning right\n", elapsed_time);
                Car_Spin_Right(i2c_file, 70, 70);
                usleep(50000);  // 0.05 seconds

            // Handle small left turn
            } else if (left2_value == LOW && right1_value == HIGH) {
                printf("[%ld ms] Adjusting left\n", elapsed_time);
                Car_Spin_Left(i2c_file, 60, 60);
                usleep(20000);  // 0.02 seconds

            // Handle small right turn
            } else if (left2_value == HIGH && right1_value == LOW) {
                printf("[%ld ms] Adjusting right\n", elapsed_time);
                Car_Spin_Right(i2c_file, 60, 60);
                usleep(20000);  // 0.02 seconds

            // Handle straight line
            } else if (left2_value == LOW && right1_value == LOW) {
                // printf("[%ld ms] Moving straight\n", elapsed_time);
                Car_Run(i2c_file, 70, 70);
            }
        }

        usleep(10000);  // 10 milliseconds delay to prevent excessive CPU usage
    }
}

int main() {
    const char *filename = "/dev/i2c-1";
    i2c_file = open_i2c_device(filename);

    if (i2c_file < 0) {
        return -1;
    }

    setup_gpio();

    // Set up the SIGINT signal handler
    signal(SIGINT, handle_sigint);

    // Record the start time
    gettimeofday(&start_time, NULL);

    // Start QR code recognition in a separate thread or process
    recognize_qr_code_thread();

    line_tracer();

    // This point will never be reached due to the infinite loop in line_tracer
    close(i2c_file);
    return 0;
}
