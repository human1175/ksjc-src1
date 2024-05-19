''' original file name : 
    ucla_________.ino '''
    
#include <ECE3.h>

uint16_t sensorValues[8];
int minVals[8] = {641, 595, 595, 664, 595, 664, 664};
int maxVals[8] = {1859, 1638, 1733, 1836, 1543, 1782, 1836, 1615};

const int left_nslp_pin = 31;
const int left_dir_pin = 29;
const int left_pwm_pin = 40;

const int right_nslp_pin = 11;
const int right_dir_pin = 30;
const int right_pwm_pin = 39;

float kp = 0.025; //0.04
float kd = 0.00001;
  int basespeed = 25;
  int control;
int error = 0;
int prevError = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(left_nslp_pin, OUTPUT);
  pinMode(left_dir_pin, OUTPUT);
  pinMode(left_pwm_pin, OUTPUT);
  digitalWrite(left_dir_pin, LOW);
  digitalWrite(right_dir_pin, LOW);
  digitalWrite(left_nslp_pin, HIGH);

  pinMode(right_nslp_pin, OUTPUT);
  pinMode(right_dir_pin, OUTPUT);
  pinMode(right_pwm_pin, OUTPUT);

  digitalWrite(right_nslp_pin, HIGH);

  ECE3_Init();
  // Serial.begin(9600);
}

void loop() {
int check = 0;
  ECE3_read_IR(sensorValues);

  if (sensorValues[0] >= 2300 and sensorValues[1] >= 2300){
    sensorValues[0] = 0;
    sensorValues[1] = 0;
    check = 1;
  }
  if (sensorValues[6] >= 2300 and sensorValues[7] >= 2300){
    sensorValues[6] = 0;
    sensorValues[7] = 0;
    check = 1;
  }
  if ((sensorValues[3] + sensorValues[4] + sensorValues[2] + sensorValues[5]) >= 2500 and check == 0) {
    sensorValues[0] = 0;
    sensorValues[1] = 0;
    sensorValues[6] = 0;
    sensorValues[7] = 0;
  }
  if ((sensorValues[2]+sensorValues[3]+sensorValues[4]+sensorValues[5])>=7000){
    analogWrite(left_pwm_pin, 0);
    analogWrite(right_pwm_pin, 0);
    return;
  }
  for (unsigned char i = 0; i < 8; i++)
  {
    if (sensorValues[i] < minVals[i]) {
      minVals[i] = sensorValues[i];
    }
  //  Serial.print((sensorValues[i] - minVals[i]) * 1000 / maxVals[i]);
    sensorValues[i] = (sensorValues[i] - minVals[i]) * 1000 / maxVals[i];
   // Serial.print('\t');
    error = (-15 * sensorValues[0] - 14 *  sensorValues[1] - 12 * sensorValues[2] - 8 * sensorValues[3]
               + 8 * sensorValues[4] + 12 * sensorValues[5] + 14 * sensorValues[6] + 15 * sensorValues[7]);
  }
  error /= 8;
 // Serial.println();
  int derivative = error - prevError;
  prevError = error;

  
  control = kp * error + kd * derivative;
 // Serial.println(control);
  analogWrite(left_pwm_pin, basespeed - control);
  analogWrite(right_pwm_pin, basespeed + control);
}#include <ECE3.h>

uint16_t sensorValues[8];
int minVals[8] = {641, 595, 595, 664, 595, 664, 664};
int maxVals[8] = {1859, 1638, 1733, 1836, 1543, 1782, 1836, 1615};

const int left_nslp_pin = 31;
const int left_dir_pin = 29;
const int left_pwm_pin = 40;

const int right_nslp_pin = 11;
const int right_dir_pin = 30;
const int right_pwm_pin = 39;

float kp = 0.025; //0.04
float kd = 0.00001;
  int basespeed = 25;
  int control;
int error = 0;
int prevError = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(left_nslp_pin, OUTPUT);
  pinMode(left_dir_pin, OUTPUT);
  pinMode(left_pwm_pin, OUTPUT);
  digitalWrite(left_dir_pin, LOW);
  digitalWrite(right_dir_pin, LOW);
  digitalWrite(left_nslp_pin, HIGH);

  pinMode(right_nslp_pin, OUTPUT);
  pinMode(right_dir_pin, OUTPUT);
  pinMode(right_pwm_pin, OUTPUT);

  digitalWrite(right_nslp_pin, HIGH);

  ECE3_Init();
  // Serial.begin(9600);
}

void loop() {
int check = 0;
  ECE3_read_IR(sensorValues);

  if (sensorValues[0] >= 2300 and sensorValues[1] >= 2300){
    sensorValues[0] = 0;
    sensorValues[1] = 0;
    check = 1;
  }
  if (sensorValues[6] >= 2300 and sensorValues[7] >= 2300){
    sensorValues[6] = 0;
    sensorValues[7] = 0;
    check = 1;
  }
  if ((sensorValues[3] + sensorValues[4] + sensorValues[2] + sensorValues[5]) >= 2500 and check == 0) {
    sensorValues[0] = 0;
    sensorValues[1] = 0;
    sensorValues[6] = 0;
    sensorValues[7] = 0;
  }
  if ((sensorValues[2]+sensorValues[3]+sensorValues[4]+sensorValues[5])>=7000){
    analogWrite(left_pwm_pin, 0);
    analogWrite(right_pwm_pin, 0);
    return;
  }
  for (unsigned char i = 0; i < 8; i++)
  {
    if (sensorValues[i] < minVals[i]) {
      minVals[i] = sensorValues[i];
    }
  //  Serial.print((sensorValues[i] - minVals[i]) * 1000 / maxVals[i]);
    sensorValues[i] = (sensorValues[i] - minVals[i]) * 1000 / maxVals[i];
   // Serial.print('\t');
    error = (-15 * sensorValues[0] - 14 *  sensorValues[1] - 12 * sensorValues[2] - 8 * sensorValues[3]
               + 8 * sensorValues[4] + 12 * sensorValues[5] + 14 * sensorValues[6] + 15 * sensorValues[7]);
  }
  error /= 8;
 // Serial.println();
  int derivative = error - prevError;
  prevError = error;

  
  control = kp * error + kd * derivative;
 // Serial.println(control);
  analogWrite(left_pwm_pin, basespeed - control);
  analogWrite(right_pwm_pin, basespeed + control);
}
