#include <PID_v1.h>
#include <U8glib.h>

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
double denature = 95; // Desired setpoint temperatures in Celsius
double annealing = 56; // Desired setpoint temperatures in Celsius
double extension = 72; // Desired setpoint temperatures in Celsius
double currentSetpoint = denature;

// Define variables to keep track of cycle and stage
int cycle;
int stage =1;

// Control parameters for PID
double Input, Output;
double Kp = 1; // Proportional gain 2
double Ki = 0; // Integral gain 5
double Kd = 0; // Derivative gain 1
PID myPID(&Input, &Output, &currentSetpoint,2,5,1,P_ON_M, DIRECT);

//Timer
unsigned long previousMillis = 0;
const long setpoint1Interval = 40000; // 10 วินาที
const long setpoint2Interval = 40000; // 30 วินาที
const long setpoint3Interval = 40000; // 30 วินาที

//LCD setting
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4 ,7); //Enable, RW, RS, RESET

void setup() {
  //Peltier
  pinMode(peltierPin, OUTPUT); //Peltier output
  //FAN pin
  pinMode(fanPin, OUTPUT);  //FAN output
  //LCD 12864 set
  u8g.begin(); //LCD 12864 begin
  // Initialize PID control
  myPID.SetMode(AUTOMATIC);
  //MAX31855
  thermocouple.begin();

   Serial.begin(9600); //serial print

}

void loop() {
  //MAX31855
  float max31855temp = thermocouple.readCelsius();  // อ่านอุณหภูมิจาก MAX31855
  if (!isnan(max31855temp)) {  // ตรวจสอบว่าค่าไม่ใช่ NaN
    Input = max31855temp - 5;  // กำหนดค่า tempRead เป็นค่าที่อ่านได้จาก MAX31855 (-5 = correction value)
  }
  
  //Read temp to PID calculator
   unsigned long periodTemp = 1000; // ระยะเวลาที่ต้องการรอ
   static unsigned long last_timeTemp = 0; // ประกาศตัวแปรเป็น static เพื่อเก็บค่าไว้ไม่ให้ reset จากการวน loop
      if (millis() - last_timeTemp > periodTemp) {
        last_timeTemp = millis(); // เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
        Serial.println(Input);
      }
  // Compute PID control
  myPID.Compute();

  //serial print
  //Serial.print("Temperature: "); 
  //Serial.println(Input); 
  //Serial.print("Cycles: "); 
  //Serial.println(); 
  //Serial.print("Stages: "); 
  //Serial.println(stage); 
  //Serial.print("PID control: "); 
  //Serial.println(Output);

  // Print current temperature, setpoint, and PID output on the LCD
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_profont12); // select front
      u8g.drawStr(0, 10, "C-Temp:"); //draw vertical position
      u8g.setPrintPos(55, 10); //draw horizontal
      u8g.print(Input);
      u8g.print(" C");

      u8g.drawStr(0, 20, "Set-Temp:"); //draw vertical position
      u8g.setPrintPos(70, 20); //draw horizontal
      u8g.print(currentSetpoint);
      u8g.print(" C");
      
      u8g.drawStr(0, 30, "PID Output:"); //draw vertical position
      u8g.setPrintPos(80, 30); //draw horizontal
      u8g.print(Output);

      u8g.drawStr(0, 40, "stage:"); //draw vertical position
      u8g.setPrintPos(80, 40); //draw horizontal
      u8g.print(stage);

      u8g.drawStr(0, 50, "cycle"); //draw vertical position
      u8g.setPrintPos(80, 50); //draw horizontal
      u8g.print(cycle);
    } while (u8g.nextPage());

    unsigned long currentMillis = millis();
    if (Input >= currentSetpoint) {
    analogWrite(peltierPin, 0);
    analogWrite(fanPin, 0);

    if (currentSetpoint == denature && currentMillis - previousMillis >= setpoint1Interval) {
      currentSetpoint = annealing;
      previousMillis = currentMillis;
      //myPID.Setpoint(currentSetpoint);
      stage=2;
    } else if (currentSetpoint == annealing && currentMillis - previousMillis >= setpoint2Interval) {
      currentSetpoint = extension;
      previousMillis = currentMillis;
      //myPID.Setpoint(currentSetpoint);
      stage=3;
    } else if (currentSetpoint == extension && currentMillis - previousMillis >= setpoint3Interval) {
      currentSetpoint = denature;
      previousMillis = currentMillis;
      //myPID.Setpoint(currentSetpoint);
      stage=1;
      cycle++;
    }
  } else {
    myPID.Compute();
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 255);
  }

  
}