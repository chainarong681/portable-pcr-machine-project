#include <PID_v1.h>
#include <U8glib.h>

//Temp value
double max31855temp;
double readTemp;

//MAX31855
#include <SPI.h>
#include "Adafruit_MAX31855.h"

// digital IO pins.
#define MAXDO   9
#define MAXCS   10
#define MAXCLK  11

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Peltier setup
int peltierPin = 3; // Digital pin connected to the Peltier module

//FAN pin (PWM control)
int fanPin = 8;  

// Setpoint temperatures
float denature = 50; // Desired setpoint temperatures in Celsius
float annealing = 45; // Desired setpoint temperatures in Celsius
float extension = 40; // Desired setpoint temperatures in Celsius

// Define variables to keep track of cycle and stage
int cycle = 0;
int stage = 0;
int totalCycles = 0;

// Control parameters for PID
double Input, Output, Setpoint;
double Kp = 2; // Proportional gain 2
double Ki = 5; // Integral gain 5
double Kd = 1; // Derivative gain 1

PID myPID(&Input, &Output, &Setpoint,2,5,1,P_ON_M, DIRECT);

//Time interval between temperature control (30 seconds)
unsigned long previousMillis = 0;
const unsigned long interval = 30000; //30 sec

//LCD setting
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4 ,7); //Enable, RW, RS, RESET

void setup() {
  //Peltier
  pinMode(peltierPin, OUTPUT); //Peltier output

  //FAN pin
  pinMode(fanPin, OUTPUT);  //FAN output

  Serial.begin(9600); //serial print

  u8g.begin(); //LCD 12864 begin

  // Initialize PID control
  myPID.SetMode(AUTOMATIC);

  //MAX31855
  thermocouple.begin();

}

void loop() {
  Input = readTemp;
  //MAX31855
  float max31855temp = thermocouple.readCelsius();  // อ่านอุณหภูมิจาก MAX31855
  if (!isnan(max31855temp)) {  // ตรวจสอบว่าค่าไม่ใช่ NaN
    readTemp = max31855temp;  // กำหนดค่า tempRead เป็นค่าที่อ่านได้จาก MAX31855
  }
  

  // Compute PID control
  myPID.Compute();

  //serial print
  Serial.print("Temperature: "); 
  Serial.println(Input); 
  Serial.print("Cycles: "); 
  Serial.println(totalCycles); 
  Serial.print("Stages: "); 
  Serial.println(stage); 
  Serial.print("PID control: "); 
  Serial.println(Output);

  // Print current temperature, setpoint, and PID output on the LCD
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_profont12); // select front
      u8g.drawStr(0, 10, "C-Temp:"); //draw vertical position
      u8g.setPrintPos(55, 10); //draw horizontal
      u8g.print(Input);
      u8g.print(" C");

      u8g.drawStr(0, 30, "Set-Temp:"); //draw vertical position
      u8g.setPrintPos(70, 30); //draw horizontal
      u8g.print(Setpoint);
      u8g.print(" C");
      
      u8g.drawStr(0, 50, "PID Output:"); //draw vertical position
      u8g.setPrintPos(80, 50); //draw horizontal
      u8g.print(Output);

      u8g.drawStr(0, 60, "stage:"); //draw vertical position
      u8g.setPrintPos(80, 60); //draw horizontal
      u8g.print(stage);
    } while (u8g.nextPage());

    ////////////////////////////////////////////////////////////////////////////////
     if (stage == 0) {
  Setpoint = denature; // Setpoint PID setting
  // Initial denaturation
  if (Input <= denature) {
    analogWrite(peltierPin, Output); // PID output is 0-255 used to control PWM of peltier
    analogWrite(fanPin, 255); // Fan speed max 225 used to control PWM
    if (Input >= denature) {
      unsigned long period1 = 10000; // ระยะเวลาที่ต้องการรอ
      static unsigned long last_time1 = 0; // ประกาศตัวแปรเป็น static เพื่อเก็บค่าไว้ไม่ให้ reset จากการวน loop
      if (millis() - last_time1 > period1) {
        last_time1 = millis(); // เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
        stage = 1; // เปลี่ยน stage เมื่อเวลาผ่านมาพอสมควร
      }
    }
  }
} 
if (stage == 1) {
  Setpoint = annealing; // Setpoint PID setting
  // Annealing
  if (Input <= annealing) {
    analogWrite(peltierPin, Output); // PID output is 0-255 used to control PWM of peltier
    analogWrite(fanPin, 255); // Fan speed max 255 used to control PWM
    unsigned long period2 = 30000; // ระยะเวลาที่ต้องการรอ
    static unsigned long last_time2 = 0; // ประกาศตัวแปรเป็น static เพื่อเก็บค่าไว้ไม่ให้ reset จากการวน loop
    if (millis() - last_time2 > period2) {
      last_time2 = millis(); // เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
      stage = 2; // เปลี่ยน stage เมื่อเวลาผ่านมาพอสมควร
    }
  }
}
if (stage == 2) {
  Setpoint = extension; // Setpoint PID setting
  // Extension
  if (Input <= extension) {
    analogWrite(peltierPin, Output); // PID output is 0-255 used to control PWM of peltier
    analogWrite(fanPin, 255); // Fan speed max 255 used to control PWM
    if (Input >= extension) {
      unsigned long period3 = 30000; // ระยะเวลาที่ต้องการรอ
      static unsigned long last_time3 = 0; // ประกาศตัวแปรเป็น static เพื่อเก็บค่าไว้ไม่ให้ reset จากการวน loop
      if (millis() - last_time3 > period3) {
        last_time3 = millis(); // เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
        stage = 0; // เปลี่ยน stage เมื่อเวลาผ่านมาพอสมควร
        totalCycles ++;
      }
    }
  }
}
}