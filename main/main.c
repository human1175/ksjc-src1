#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdlib.h>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"

int i2c_file;  // Global variable to store the I2C file descriptor

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
        
        // Print sensor values for debugging
        printf("Left1: %d, Left2: %d, Right1: %d, Right2: %d\n", left1_value, left2_value, right1_value, right2_value);

        // Simple line tracing logic
        if (left1_value == 0 && right1_value == 0) {
            if (left2_value == 1 && right2_value ==1){
                Car_Run(i2c_file, 20, 20);
            }
            else if (left2_value == 0 && right2_value ==1){
                Car_Run(i2c_file, 10, 0);
            }
            else if (left2_value == 1 && right2_value ==0){
                Car_Run(i2c_file, 0, 10);
            }
            else{
                //4개 다 검정이 인식되는 경우인데, 이런 경우는 발생안하는게 베스트긴함. 일단 비움.
            }
        }
        else if (left1_value == 0 && right1_value == 1){
            if (left2_value == 0){
                Car_Run(i2c_file, 20, 0);
            }
            else{
                Car_Run(i2c_file, 10, 0);
            }
        }

        else if (left1_value == 1 && right1_value == 0){
            if (right2_value == 0){
                Car_Run(i2c_file, 0, 20);
            }
            else{
                Car_Run(i2c_file, 0, 10);
            }
        }

        else if (left1_value == 1 && right1_value == 1){
            if (right2_value == 0){
                Car_Run(i2c_file, 0, 30);
            }
            else if (left2_value==0){
                Car_Run(i2c_file, 30, 0);
            }
        }

        
        usleep(100000);  // Delay to prevent excessive CPU usage
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