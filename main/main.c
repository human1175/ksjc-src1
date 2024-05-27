#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdlib.h>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"

int i2c_file;  // Global variable to store the I2C file descriptor
double previous_error = 0.0;
double integral = 0.0;

// PID constants
#define KP 1.0
#define KI 0.0
#define KD 0.1

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

        // Calculate the error
        double error = (left1_value * -3 + left2_value * -1 + right1_value * 1 + right2_value * 3);

        // Proportional term
        double P = KP * error;

        // Integral term
        integral += error;
        double I = KI * integral;

        // Derivative term
        double derivative = error - previous_error;
        double D = KD * derivative;

        // Calculate the control variable
        double control = P + I + D;
        previous_error = error;

        // Determine motor speeds based on the control variable
        int base_speed = 70;
        int left_speed = base_speed + control;
        int right_speed = base_speed - control;

        // Limit the motor speeds to valid range
        if (left_speed > 100) left_speed = 100;
        if (left_speed < 0) left_speed = 0;
        if (right_speed > 100) right_speed = 100;
        if (right_speed < 0) right_speed = 0;

        // Apply the motor speeds
        Car_Run(i2c_file, left_speed, right_speed);

        // Print sensor values for debugging
        printf("Left1: %d, Left2: %d, Right1: %d, Right2: %d, Control: %f\n", left1_value, left2_value, right1_value, right2_value, control);

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