#include <stdio.h>
#include <unistd.h>
#include "yb_pcb_car.h"

int main() {
    const char *filename = "/dev/i2c-1";
    int i2c_file = open_i2c_device(filename);

    if (i2c_file < 0) {
        return -1;
    }

    // Example usage
    Car_Run(i2c_file, 100, 100);  // Run forward
    sleep(1);
    Car_Back(i2c_file, 100, 100);  // Run backward
    sleep(1);
    Car_Left(i2c_file, 50, 100);  // Turn left
    sleep(1);
    Car_Right(i2c_file, 100, 50);  // Turn right
    sleep(1);
    Car_Spin_Left(i2c_file, 100, 100);  // Spin left
    sleep(1);
    Car_Spin_Right(i2c_file, 100, 100);  // Spin right
    sleep(1);
    Ctrl_Servo(i2c_file, 1, 90);  // Control servo
    sleep(1);
    Car_Stop(i2c_file);

    close(i2c_file);

    return 0;
}
