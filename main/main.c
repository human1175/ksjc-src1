#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdlib.h>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"

int i2c_file;  // Global variable to store the I2C file descriptor
int base_speed = 100;  // Base speed variable

// Signal handler to stop the motors and clean up
void handle_sigint(int sig) {
    Car_Stop(i2c_file);
    close(i2c_file);
    printf("Motors stopped and I2C file closed. Exiting...\n");
    exit(0);
}

// Function to read sensors and control the car accordingly
void line_tracer() {
    while (1) {
        int left1_value = digitalRead(LEFT1_PIN);
        int left2_value = digitalRead(LEFT2_PIN);
        int right1_value = digitalRead(RIGHT1_PIN);
        int right2_value = digitalRead(RIGHT2_PIN);

        int sharp_turn_speed_1 = 70;
        int sharp_turn_speed_2 = 30;
        
        // Print sensor values for debugging
        printf("Left1: %d, Left2: %d, Right1: %d, Right2: %d\n", left1_value, left2_value, right1_value, right2_value);

        // Implement the logic based on sensor values
        if ((left1_value == 1 || left2_value == 1) && right2_value == 1) {
            Car_Spin_Right(i2c_file, base_speed, base_speed / 2);
            usleep(200000);  // 0.2 seconds

        // Handle sharp and right angles
        } else if (left1_value == 1 && (right1_value == 1 || right2_value == 1)) {
            Car_Spin_Left(i2c_file, base_speed / 2, base_speed);
            usleep(200000);  // 0.2 seconds

        // Detect most left
        } else if (left1_value == 1) {
            Car_Spin_Left(i2c_file, base_speed, base_speed);
            usleep(50000);  // 0.05 seconds

        // Detect most right
        } else if (right2_value == 1) {
            Car_Spin_Right(i2c_file, base_speed, base_speed);
            usleep(50000);  // 0.05 seconds

        // Handle small left turn
        } else if (left2_value == 1 && right1_value == 0) {
            Car_Spin_Left(i2c_file, base_speed * 0.8, base_speed * 0.8);
            usleep(20000);  // 0.02 seconds

        // Handle small right turn
        } else if (left2_value == 0 && right1_value == 1) {
            Car_Spin_Right(i2c_file, base_speed * 0.8, base_speed * 0.8);
            usleep(20000);  // 0.02 seconds

        // Handle straight line
        } else if (left2_value == 1 && right1_value == 1) {
            Car_Run(i2c_file, base_speed, base_speed);
        } else {
            Car_Stop(i2c_file);
        }

        usleep(10000);  // Delay to prevent excessive CPU usage
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

    line_tracer();

    // This point will never be reached due to the infinite loop in line_tracer
    close(i2c_file);
    return 0;
}