#ifndef TRACKING_SENSOR_H
#define TRACKING_SENSOR_H

#include <stdint.h>
#include <wiringPi.h>  // Include wiringPi.h for digitalRead and pinMode functions

#define LEFT1_PIN 27  // Define BCM pin for Left 1
#define LEFT2_PIN 22  // Define BCM pin for Left 2
#define RIGHT1_PIN 17  // Define BCM pin for Right 1
#define RIGHT2_PIN 4   // Define BCM pin for Right 2

void setup_gpio();
void read_tracking_sensors();

#endif // TRACKING_SENSOR_H
