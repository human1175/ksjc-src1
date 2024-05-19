''' original file name : 
    Line_Tracer_Projeect_Kp_Kd__Final_.ino '''
    
#include <ECE3.h>

uint16_t sensorValues[8];
// (로봇 시점 기준) right -> left, S1 (0) -> S8 (7)

const int left_nslp_pin=31; // nslp ==> awake & ready for PWM
const int left_dir_pin=29;
const int left_pwm_pin=40;

const int right_nslp_pin=11;
const int right_dir_pin=30;
const int right_pwm_pin=39;

const int LED_RF = 41;
const int LED_LF = 51;

float Kp = 0.02;
float Kd = 0.0001;

//declarations
  float minValue[8] = {790.9, 664.0, 861.9, 687.0, 743.9, 687.0, 782.3, 801.6};
  float maxValue[8] = {1709.1,1836.0, 1638.1, 1813.0, 1756.1, 1813.0, 1717.7, 1698.4};
  float fusion_output = 0;
  float prev_fusion_output = 0;
  float norm[8];
  float f[8] = {-15, -14, -12 , -8 ,8 ,12 ,14 , 15};

///////////////////////////////////
void setup() {
// put your setup code here, to run once:
  pinMode(left_nslp_pin,OUTPUT);
  pinMode(left_dir_pin,OUTPUT);
  pinMode(left_pwm_pin,OUTPUT);

  pinMode(right_nslp_pin,OUTPUT);
  pinMode(right_dir_pin,OUTPUT);
  pinMode(right_pwm_pin,OUTPUT);

  digitalWrite(left_dir_pin,LOW);
  digitalWrite(left_nslp_pin,HIGH);

  digitalWrite(right_dir_pin,LOW);
  digitalWrite(right_nslp_pin,HIGH);
//  digitalWrite(left_nslp_pin,LOW);

  pinMode(LED_RF, OUTPUT);
  pinMode(LED_LF, OUTPUT);
  
  ECE3_Init();

// set the data rate in bits/second for serial data transmission
  Serial.begin(9600); 
  delay(2000); //Wait 2 seconds before starting 
}

void loop() {
  ECE3_read_IR(sensorValues);
  
  // put your main code here, to run repeatedly: 
  int leftSpd = 30;
  int rightSpd = 30;
  int blackcount_variable = 0;

  for (int i = 0; i < 8; i++)
  {
    if (sensorValues[i] >= 1500)
    {
     blackcount_variable += 1;
    }
  }

  if (sensorValues[2] >= 1500 or sensorValues[3] >= 1500 or sensorValues[4] >= 1500 or sensorValues[5] >= 1500)
  {
  sensorValues[0] = 0;
  sensorValues[1] = 0;
  sensorValues[6] = 0;
  sensorValues[7] = 0;
  }
  
  prev_fusion_output = fusion_output;
    
  for (unsigned char i = 0; i < 8; i++)
  {
    norm[i] = (sensorValues[i]-minValue[i]);
    norm[i] = norm[i]*1000.0;
    norm[i] = norm[i]/maxValue[i];
    fusion_output += 0.5 * f[i]* norm[i];
  }
  fusion_output = fusion_output/4;
  //fusion_output = 500 * pow(abs(fusion_output),2/3) * fusion_output / abs(fusion_output);

  leftSpd -= (Kd) * (fusion_output - prev_fusion_output) + Kp * fusion_output ;
  rightSpd += (Kd) * (fusion_output - prev_fusion_output) + Kp * fusion_output ;

  if (blackcount_variable > 5)
  {
  leftSpd = 0;
  rightSpd = 0;
  }

  analogWrite(left_pwm_pin,leftSpd);
  analogWrite(right_pwm_pin,rightSpd);

}
