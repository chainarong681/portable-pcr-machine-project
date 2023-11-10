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
double WARM_TEMP = 40; // Desired setpoint temperatures in Celsius
double RT_TEMP = 50; // Desired setpoint temperatures in Celsius
double INITIAL_DENATURE_TEMP = 95; // Desired setpoint temperatures in Celsius
double DENATURE_TEMP = 95; // Desired setpoint temperatures in Celsius
double ANNEALING_TEMP = 56; // Desired setpoint temperatures in Celsius
double EXTENTION_TEMP = 72; // Desired setpoint temperatures in Celsius
double COOLING_TEMP = 40; // Desired setpoint temperatures in Celsius
const float tolerance = 0.5;  // กำหนดค่าความเป็นไปได้ (+0.5, -0.5)
//Timer
unsigned long int WARM_TEMP_TIME = 60000;
unsigned long int RT_TEMP_TIME = 60000;
unsigned long int INITIAL_DENATURE_TIME = 60000;
unsigned long int DENATURE_TEMP_TIME = 60000;
unsigned long int ANNEALING_TEMP_TIME = 60000;
unsigned long int EXTENTION_TEMP_TIME = 60000;
unsigned long int COOLING_TEMP_TIME = 60000;

// Define variables to keep track of cycle and stage
int NUMBER_CYCLE =0;
int STAGE =1;
unsigned long previousMillis = 0;
unsigned long interval = 0;

// Control parameters for PID
double Input, Output, Setpoint;
double Kp = 0; // Proportional gain 0
double Ki = 0; // Integral gain 
double Kd = 0; // Derivative gain 
PID myPID(&Input, &Output, &Setpoint,2,5,1,P_ON_M, DIRECT);

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
  
  //Warm temp
  Setpoint = 40;
}

void loop() {
  //MAX31855
  float max31855temp = thermocouple.readCelsius();  // อ่านอุณหภูมิจาก MAX31855
  if (!isnan(max31855temp)) {  // ตรวจสอบว่าค่าไม่ใช่ NaN
    Input = max31855temp;  // กำหนดค่า tempRead เป็นค่าที่อ่านได้จาก MAX31855 (? = correction value)
  }
  
  //Read temp to PID calculator
   unsigned long periodTemp = 1000; // ระยะเวลาที่ต้องการรอ
   static unsigned long last_timeTemp = 0; // ประกาศตัวแปรเป็น static เพื่อเก็บค่าไว้ไม่ให้ reset จากการวน loop
      if (millis() - last_timeTemp > periodTemp) {
        last_timeTemp = millis(); // เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
        Serial.println(Input);
      }
  // Compute PID control
  //myPID.Compute();

  //Serial print
  //printSerial();

  // Print current temperature, setpoint, and PID output on the LCD
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_profont12); // select front
      u8g.drawStr(0, 10, "Read:"); //draw vertical position
      u8g.setPrintPos(55, 10); //draw horizontal
      u8g.print(Input);
      u8g.print(" C");

      u8g.drawStr(0, 20, "Set-Temp:"); //draw vertical position
      u8g.setPrintPos(70, 20); //draw horizontal
      u8g.print(Setpoint);
      u8g.print(" C");
      
      u8g.drawStr(0, 30, "PID Output:"); //draw vertical position
      u8g.setPrintPos(80, 30); //draw horizontal
      u8g.print(Output);

      u8g.drawStr(0, 40, "stage:"); //draw vertical position
      u8g.setPrintPos(80, 40); //draw horizontal
      u8g.print(STAGE);

      u8g.drawStr(0, 50, "cycle"); //draw vertical position
      u8g.setPrintPos(80, 50); //draw horizontal
      u8g.print(NUMBER_CYCLE);
    } while (u8g.nextPage());

    instrumentRun();
 
    
    

}

void instrumentRun(){
  //PCR control
  unsigned long currentMillis = millis();
  interval = currentMillis - previousMillis;

  switch (STAGE) {
    case 1:
      // ควบคุมอุณหภูมิ 40 องศาเซลเซียส = warm
      myPID.Compute();
      runInstrument();
      if (Input > Setpoint && interval >= 120000) {
        STAGE = 2;
        previousMillis = currentMillis;
      }
      break;

    case 2:
      // ควบคุมอุณหภูมิ 50 องศาเซลเซียส
      Setpoint = 50;
      myPID.Compute();
      runInstrument();
      if (Input > Setpoint && interval >= 30000) {
        STAGE = 3;
        previousMillis = currentMillis;
      }
      break;

    case 3:
      // ควบคุมอุณหภูมิ 95 องศาเซลเซียส
      Setpoint = 95;
      myPID.Compute();
      runInstrument();
      if (Input > Setpoint && interval >= 30000) {
        STAGE = 4;
        previousMillis = currentMillis;
      }
      break;

    case 4:
      // ควบคุมอุณหภูมิ 90 องศาเซลเซียส
      Setpoint = 95;
      myPID.Compute();
      runInstrument();
      if (Input > Setpoint && interval >= 30000) {
        STAGE = 5;
        previousMillis = currentMillis;
      }
      break;

    case 5:
      // ควบคุมอุณหภูมิ 60 องศาเซลเซียส
      Setpoint = 60;
      myPID.Compute();
      runInstrument();
      if (Input < Setpoint && interval >= 30000) {
        STAGE = 6;
        previousMillis = currentMillis;
      }
      break;

    case 6:
      // ควบคุมอุณหภูมิ 72 องศาเซลเซียส
      Setpoint = 72;
      myPID.Compute();
      runInstrument();
      if (Input > Setpoint && interval >= 30000) {
        STAGE = 4; // กลับไปสู่สถานะ 1
        NUMBER_CYCLE++; // เพิ่มจำนวนรอบ
        previousMillis = currentMillis;
      }

      if (NUMBER_CYCLE== 35) {
        // ทำสิ้นสุดการทำงานหลักและหยุดการวนลูป
        while (true) {
          // วงจรนี้จะหยุดทำงาน
        }
      }
      break;
  }
}

//Run temp
void runInstrument(){
if (Input >= Setpoint) {
    analogWrite(peltierPin, 0);
    analogWrite(fanPin, 0);
  } else {
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 255);
  }
}

//stady state read temp
void stadyState(){
    for (int i = 0; i < 5; i++) {
    if (Input <= tolerance) {
      // ค่าอยู่ในช่วงที่กำหนด
      //Serial.println("ค่าอยู่ในช่วงที่กำหนด");
      // ทำการประมวลผลตามคำสั่งที่ต้องการ
    } else {
      // ค่าไม่อยู่ในช่วงที่กำหนด
      //Serial.println("ค่าไม่อยู่ในช่วงที่กำหนด");
      // ไม่ทำการประมวลผลหรือทำอย่างอื่นที่ต้องการ
    }

    delay(1000);  // หน่วงเวลา 1 วินาที
  }

  // สิ้นสุดการตรวจสอบ 5 ครั้ง
  while (true) {
    // ทำอะไรก็ตามที่ต้องการทำหลังจากการตรวจสอบ 5 ครั้ง
  }
}
