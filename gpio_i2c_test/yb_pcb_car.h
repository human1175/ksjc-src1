#ifndef YB_PCB_CAR_H
#define YB_PCB_CAR_H

#include <stdint.h>

// Function declarations for motor control
int open_i2c_device(const char *filename);
void write_u8(int file, uint8_t reg, uint8_t data);
void write_reg(int file, uint8_t reg);
void write_array(int file, uint8_t reg, uint8_t *data, size_t length);
void Ctrl_Car(int file, uint8_t l_dir, uint8_t l_speed, uint8_t r_dir, uint8_t r_speed);
void Control_Car(int file, int speed1, int speed2);
void Car_Run(int file, uint8_t speed1, uint8_t speed2);
void Car_Stop(int file);
void Car_Back(int file, uint8_t speed1, uint8_t speed2);
void Car_Left(int file, uint8_t speed1, uint8_t speed2);
void Car_Right(int file, uint8_t speed1, uint8_t speed2);
void Car_Spin_Left(int file, uint8_t speed1, uint8_t speed2);
void Car_Spin_Right(int file, uint8_t speed1, uint8_t speed2);
void Ctrl_Servo(int file, uint8_t id, uint8_t angle);

#endif // YB_PCB_CAR_H
