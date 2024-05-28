#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include "yb_pcb_car.h"

#define I2C_ADDR 0x16

int open_i2c_device(const char *filename) {
    int file;

    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        return -1;
    }

    if (ioctl(file, I2C_SLAVE, I2C_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        close(file);
        return -1;
    }

    return file;
}

void write_u8(int file, uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};

    if (write(file, buffer, 2) != 2) {
        perror("write_u8 I2C error");
    }
}

void write_reg(int file, uint8_t reg) {
    if (write(file, &reg, 1) != 1) {
        perror("write_reg I2C error");
    }
}

void write_array(int file, uint8_t reg, uint8_t *data, size_t length) {
    uint8_t buffer[length + 1];
    buffer[0] = reg;
    for (size_t i = 0; i < length; i++) {
        buffer[i + 1] = data[i];
    }

    if (write(file, buffer, length + 1) != length + 1) {
        perror("write_array I2C error");
    }
}

void Ctrl_Car(int file, uint8_t l_dir, uint8_t l_speed, uint8_t r_dir, uint8_t r_speed) {
    uint8_t reg = 0x01;
    uint8_t data[4] = {l_dir, l_speed, r_dir, r_speed};

    write_array(file, reg, data, 4);
}

void Control_Car(int file, int speed1, int speed2) {
    uint8_t dir1 = (speed1 < 0) ? 0 : 1;
    uint8_t dir2 = (speed2 < 0) ? 0 : 1;
    uint8_t speed1_abs = (uint8_t)fabs(speed1);
    uint8_t speed2_abs = (uint8_t)fabs(speed2);

    Ctrl_Car(file, dir1, speed1_abs, dir2, speed2_abs);
}

void Car_Run(int file, uint8_t speed1, uint8_t speed2) {
    Ctrl_Car(file, 1, speed1, 1, speed2);
}

void Car_Stop(int file) {
    uint8_t reg = 0x02;
    write_u8(file, reg, 0x00);
}

void Car_Back(int file, uint8_t speed1, uint8_t speed2) {
    Ctrl_Car(file, 0, speed1, 0, speed2);
}

void Car_Left(int file, uint8_t speed1, uint8_t speed2) {
    Ctrl_Car(file, 0, speed1, 1, speed2);
}

void Car_Right(int file, uint8_t speed1, uint8_t speed2) {
    Ctrl_Car(file, 1, speed1, 0, speed2);
}

void Car_Spin_Left(int file, uint8_t speed1, uint8_t speed2) {
    Ctrl_Car(file, 0, speed1, 1, speed2);
}

void Car_Spin_Right(int file, uint8_t speed1, uint8_t speed2) {
    Ctrl_Car(file, 1, speed1, 0, speed2);
}

void Ctrl_Servo(int file, uint8_t id, uint8_t angle) {
    uint8_t reg = 0x03;
    uint8_t data[2] = {id, angle};

    if (angle < 0) {
        angle = 0;
    } else if (angle > 180) {
        angle = 180;
    }

    write_array(file, reg, data, 2);
}
