#include <stdio.h>
#include <wiringPi.h>
#include "tracking_sensor.h"

// GPIO initialization function
void setup_gpio() {
    if (wiringPiSetupGpio() == -1) {
        perror("Failed to initialize WiringPi with GPIO numbering");
        return;
    }
    pinMode(LEFT1_PIN, INPUT);
    pinMode(LEFT2_PIN, INPUT);
    pinMode(RIGHT1_PIN, INPUT);
    pinMode(RIGHT2_PIN, INPUT);
    printf("GPIO setup completed.\n");
}

// Function to read tracking sensor data and print it
void read_tracking_sensors() {
    printf("Tracking sensor test start...\n");

    while (1) {
        int left1_value = digitalRead(LEFT1_PIN);
        int left2_value = digitalRead(LEFT2_PIN);
        int right1_value = digitalRead(RIGHT1_PIN);
        int right2_value = digitalRead(RIGHT2_PIN);

        printf("Left1: %d\n", left1_value);
        printf("Left2: %d\n", left2_value);
        printf("Right1: %d\n", right1_value);
        printf("Right2: %d\n", right2_value);
        printf("---\n");

        delay(1000);  // Delay for 1 second
    }
}
