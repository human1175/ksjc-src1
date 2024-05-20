#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include "yb_pcb_car.h"
#include "tracking_sensor.h"

// Function to read sensors and control the car accordingly
void line_tracer(int i2c_file) {
    while (1) {
        int left1_value = digitalRead(LEFT1_PIN);
        int left2_value = digitalRead(LEFT2_PIN);
        int right1_value = digitalRead(RIGHT1_PIN);
        int right2_value = digitalRead(RIGHT2_PIN);

        // Print sensor values for debugging
        printf("Left1: %d, Left2: %d, Right1: %d, Right2: %d\n", left1_value, left2_value, right1_value, right2_value);

        // Simple line tracing logic
        if (left1_value == 0 && right1_value == 0) {
            // Move forward
            Car_Run(i2c_file, 100, 100);
        } else if (left1_value == 0) {
            // Turn left
            Car_Left(i2c_file, 100, 100);
        } else if (right1_value == 0) {
            // Turn right
            Car_Right(i2c_file, 100, 100);
        } else {
            // Stop
            Car_Stop(i2c_file);
        }

        usleep(100000);  // Delay to prevent excessive CPU usage
    }
}

int main() {
    const char *filename = "/dev/i2c-1";
    int i2c_file = open_i2c_device(filename);

    if (i2c_file < 0) {
        return -1;
    }

    setup_gpio();
    line_tracer(i2c_file);
    Car_Stop(i2c_file);
    close(i2c_file);
    return 0;
}
