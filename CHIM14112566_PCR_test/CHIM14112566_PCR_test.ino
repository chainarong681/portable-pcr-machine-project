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
double ANNEALING_TEMP = 50; // Desired setpoint temperatures in Celsius
double EXTENTION_TEMP = 60; // Desired setpoint temperatures in Celsius
double FINAL_EXTENTION_TEMP = 60; // Desired setpoint temperatures in Celsius

//Time
int WARM_TEMP_TIME = 120; //value = sec
int RT_TEMP_TIME = 900; //value = sec
int INITIAL_DENATURE_TIME = 30; //value = sec
int DENATURE_TEMP_TIME = 10; //value = sec
int ANNEALING_TEMP_TIME = 45; //value = sec
int EXTENTION_TEMP_TIME = 60; //value = sec
int FINAL_EXTENTION_TEMP_TIME = 600; //value = sec

// Define variables to keep track of cycle and stage and Other
int NUMBER_CYCLE =0;
int NUMBER_CYCLE_setting =35;
int STAGE =1;


//Steady temp state constant
int measurementsCount = 0;
bool STATUS_CHECK = false;

// Timer PCR step
unsigned long period = 1000; //ระยะเวลาที่ต้องการรอ 1 วินาที
unsigned long last_time = 0; //ประกาศตัวแปรเป็น global เพื่อเก็บค่าไว้ไม่ให้ reset จากการวนloop
int TIME_COUNT = 0;
int TIMER_STEP = 0;
int TIME_CHANGE;

// Control parameters for PID
double Input, Output, Setpoint;
double Kp = 1; // Proportional gain 1
double Ki = 6; // Integral gain 6
double Kd = 2; // Derivative gain 2
//P_ON_M specifies that Proportional on Measurement be used
//P_ON_E (Proportional on Error) is the default behavior
//PID myPID(&Input, &Output, &Setpoint,2,5,1,P_ON_M, DIRECT);
//PID myPID(&Input, &Output, &Setpoint,2,5,1,P_ON_E, DIRECT);
//PID myPID(&Input, &Output, &Setpoint,2,5,1, DIRECT);
PID myPID(&Input, &Output, &Setpoint,2,5,1,P_ON_E, DIRECT);

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
  
  //Start Warm temp
  Setpoint = 40;

  //time change หยุดการจับเวลา
  TIME_CHANGE =0;
}

void loop() {
  //MAX31855
  float max31855temp = thermocouple.readCelsius();  // อ่านอุณหภูมิจาก MAX31855
  if (!isnan(max31855temp)) {  // ตรวจสอบว่าค่าไม่ใช่ NaN
    Input = max31855temp - 1;  // กำหนดค่า tempRead เป็นค่าที่อ่านได้จาก MAX31855 (-1 = correction value)
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

      u8g.drawStr(0, 60, "Status"); //draw vertical position
      u8g.setPrintPos(82, 60); //draw horizontal
      u8g.print(TIME_COUNT);
    } while (u8g.nextPage());
    
    //Run
    instrumentRun();
    stadyState();
    
    
 
}

void instrumentRun() {
  //PCR control

  switch (STAGE) {
    case 1:
      Setpoint = WARM_TEMP;
      TIMER_STEP = WARM_TEMP_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          STAGE = 2;
          TIME_CHANGE =0;
        }
      }
      break;

    case 2:
      Setpoint = RT_TEMP;
      TIMER_STEP = RT_TEMP_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          STAGE = 3;
          TIME_CHANGE =0;
        }
      }
      break;

    case 3:
      Setpoint = INITIAL_DENATURE_TEMP;
      TIMER_STEP = INITIAL_DENATURE_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          STAGE = 4;
          TIME_CHANGE =0;
        }
      }
      break;

    case 4:
      Setpoint = DENATURE_TEMP;
      TIMER_STEP = DENATURE_TEMP_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          STAGE = 5;
          TIME_CHANGE =0;
        }
      }
      break;

    case 5:
      Setpoint = ANNEALING_TEMP;
      TIMER_STEP = ANNEALING_TEMP_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          STAGE = 6;
          TIME_CHANGE =0;
        }
      }
      break;

    case 6:
      Setpoint = EXTENTION_TEMP;
      TIMER_STEP = EXTENTION_TEMP_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          TIME_CHANGE =0;
          NUMBER_CYCLE++; // เพิ่มจำนวนรอบ
          if (NUMBER_CYCLE < NUMBER_CYCLE_setting) {
            STAGE = 4; // กลับไปสู่สถานะ 4
          } else if (NUMBER_CYCLE >= NUMBER_CYCLE_setting){
            STAGE = 7; // กลับไปสู่สถานะ cooling
          }
        }
      }
      break;

      case 7:
      Setpoint = FINAL_EXTENTION_TEMP;
      TIMER_STEP = FINAL_EXTENTION_TEMP_TIME;
      runInstrument();
      stadyState();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          TIME_CHANGE =0;
          TIME_COUNT = 0;
          NUMBER_CYCLE = NUMBER_CYCLE ; // เพิ่มจำนวนรอบ
        }
      }
      
      if (NUMBER_CYCLE == NUMBER_CYCLE_setting) {
        // ทำสิ้นสุดการทำงานหลักและหยุดการวนลูป
        analogWrite(peltierPin, 0); //ปรกติจะเท่ากับ 0
        analogWrite(fanPin, 0);
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
    analogWrite(peltierPin, 0); //ปรกติจะเท่ากับ 0
    analogWrite(fanPin, 0);
  } else {
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 255);
  }
}

void stadyState() {
  static unsigned long previousStadyState = 0;
  const long intervalStadyState = 500;
  unsigned long currentMillisStadyState = millis();

  if (Input <= Setpoint + 0.5 && Input >= Setpoint - 0.5) {
    if (currentMillisStadyState - previousStadyState >= intervalStadyState) {
      measurementsCount++;
      previousStadyState = currentMillisStadyState;

      if (measurementsCount >= 10) {
        STATUS_CHECK = true;
        measurementsCount = 0; // รีเซ็ต measurementsCount เมื่อครบ 10 ครั้ง
      }
    }
  }
}

void timer(){
  if( millis() - last_time > period) {
    TIME_COUNT++;
     last_time = millis(); //เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period 
     if (TIME_COUNT >= TIMER_STEP){
      TIME_COUNT = 0;
      TIME_CHANGE = 1;
     }
 }
}