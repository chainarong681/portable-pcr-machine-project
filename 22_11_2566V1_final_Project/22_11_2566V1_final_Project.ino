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
double WARM_TEMP = 40.0; // Desired setpoint temperatures in Celsius
double RT_TEMP = 50.0; // Desired setpoint temperatures in Celsius
double INITIAL_DENATURE_TEMP = 94.0; // Desired setpoint temperatures in Celsius
double DENATURE_TEMP = 94.0; // Desired setpoint temperatures in Celsius
double ANNEALING_TEMP = 55.0; // Desired setpoint temperatures in Celsius
double EXTENTION_TEMP = 72.0; // Desired setpoint temperatures in Celsius
double FINAL_EXTENTION_TEMP = 72.0; // Desired setpoint temperatures in Celsius
//Time วินาที
int WARM_TEMP_TIME = 60; //120
int RT_TEMP_TIME = 0; //1800
int INITIAL_DENATURE_TIME = 120; //120
int DENATURE_TEMP_TIME = 30; //15
int ANNEALING_TEMP_TIME = 45; //45
int EXTENTION_TEMP_TIME = 90; //45
int COOLING_TEMP_TIME = 420; //420

// Define variables to keep track of cycle and stage and Other
int NUMBER_CYCLE =0;
int SET_CYCLE = 35;
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
double Kp = 20; // Proportional gain 40
double Ki = 4; // Integral gain 3
double Kd = 0.5; // Derivative gain 1
PID myPID(&Input, &Output, &Setpoint,Kp,Ki,Kd,P_ON_M, DIRECT);

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
    float Input_notnan = max31855temp; //ส่งค่าที่ทำการวัด

    Input = Input_notnan;
     // แก้ไขค่า Input ตามช่วงอุณหภูมิ
    if (Input >= 10 && Input < 45) {
      //แก้ WARM_TEMP = -0.5
      Input -= 0.5;
    } else if (Input >= 45 && Input < 55) {
      //แก้  RT_TEMP = -1
      Input -= 1;
    } else if (Input >= 55 && Input < 65) {
      //แก้  ANNEALING_TEMP = -1
      Input -= 1;
    } else if (Input >= 65 && Input < 80) {
      //แก้ EXTENTION_TEMP และ FINAL_EXTENTION_TEMP = -2.5
      Input -= 2.5;
    } else if (Input >= 80 && Input <= 100) {
      //แก้  INITIAL_DENATURE_TEMP และ DENATURE_TEMP  = -4 เดิม -6
      Input -= 4;
    }
  }

  //Serial print
   unsigned long periodTemp = 5000; // ระยะเวลาที่ต้องการรอ
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
      u8g.print(" / ");
      u8g.print(SET_CYCLE);

      u8g.drawStr(0, 60, "Time/Delay"); //draw vertical position
      u8g.setPrintPos(82, 60); //draw horizontal
      u8g.print(TIME_COUNT);
      u8g.print(" / ");
      u8g.print(measurementsCount);
    } while (u8g.nextPage());

    //Run
    instrumentRun();
    //stadyState();



}

void instrumentRun() {
  //PCR control
  switch (STAGE) {
    case 1:
      Setpoint = WARM_TEMP;
      TIMER_STEP = WARM_TEMP_TIME;
      runInstrument_RT_WARM();
      stadyState_anneling();
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
      runInstrument_RT_WARM();
      stadyState_anneling();
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
      runInstrument_Annealing();
      stadyState_anneling();
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
      runInstrument_Extention();
      stadyState_anneling();
      if (STATUS_CHECK == true) {
        timer();
        if (TIME_CHANGE == 1){
          STATUS_CHECK = false;
          TIME_CHANGE =0;
          NUMBER_CYCLE++; // เพิ่มจำนวนรอบ
          if (NUMBER_CYCLE < SET_CYCLE) {
            STAGE = 4; // กลับไปสู่สถานะ 4
          } else if (NUMBER_CYCLE >= SET_CYCLE){
            STAGE = 7; // กลับไปสู่สถานะ cooling
          }
        }
      }
      break;

      case 7:
      Setpoint = FINAL_EXTENTION_TEMP;
      TIMER_STEP = COOLING_TEMP_TIME;
      runInstrument_Extention();
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

      if (NUMBER_CYCLE == SET_CYCLE) {
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
//Protect >=105
if ((Input >= Setpoint) || (Input >= 105)) {
    analogWrite(peltierPin, 0); //ปรกติจะเท่ากับ 0
    analogWrite(fanPin, 128); //ปรกติ 80
  } else {
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 255);
  }
}

void runInstrument_RT_WARM(){
//Protect >=105
if ((Input >= Setpoint) || (Input >= 105)) {
    analogWrite(peltierPin, 0); //ปรกติจะเท่ากับ 0
    analogWrite(fanPin, 128); //ปรกติ 80
  } else {
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 128); //ทดสอบที่ความเร็วครึ่งนึง
  }
}

//Run temp
void runInstrument_Extention(){
//Protect >=105
if ((Input >= Setpoint) || (Input >= 105)) {
    analogWrite(peltierPin, 0); //ปรกติจะเท่ากับ 0
    analogWrite(fanPin, 128); //ปรกติ 80
  } else {
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 200); //ทดสอบที่ความเร็ว
  }
}

//Run temp
void runInstrument_Annealing(){
//Protect >=105
if ((Input >= Setpoint) || (Input >= 105)) {
    analogWrite(peltierPin, 0); //ปรกติจะเท่ากับ 0
    analogWrite(fanPin, 128); //ปรกติ 80
    //ลดอุณหภูมิให้เร็วที่สุดก่อนถึงค่า Setpoint
    if (Input >= Setpoint + 1.5){
      analogWrite(fanPin, 255); //ทดสอบแล้ว 255 เพื่อลดอุณหภูมิให้เร็วที่สุด
    }
  } else {
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 128); //ทดสอบที่ความเร็วครึ่งนึง
  }
}

void stadyState() {
  static unsigned long previousStadyStateD = 0;
  const long intervalStadyStateD = 1000;
  unsigned long currentMillisStadyStateD = millis();

  if ((Input <= Setpoint + 0.25) && (Input >= Setpoint - 0.25) && (STATUS_CHECK == false)) {
    if (currentMillisStadyStateD - previousStadyStateD >= intervalStadyStateD) {
      measurementsCount++;
      previousStadyStateD = currentMillisStadyStateD;

      if (measurementsCount >= 12) { //ให้ค่าเข่าใกล้ค่าคงที่มากที่สุดก่อนเพื่อจะเริ่มจับเวลา
        STATUS_CHECK = true;
        measurementsCount = 0; // รีเซ็ต measurementsCount เมื่อครบ 12 ครั้ง ใช้เวลาเพิ่ม 12 วินาที
      }
    }
  }
}

void stadyState_anneling(){
  static unsigned long previousStadyStateA = 0;
  const long intervalStadyStateA = 1000;
  unsigned long currentMillisStadyStateA = millis();

  if ((Input <= Setpoint + 0.25) && (Input >= Setpoint - 0.25) && (STATUS_CHECK == false)) {
    if (currentMillisStadyStateA - previousStadyStateA >= intervalStadyStateA) {
      measurementsCount++;
      previousStadyStateA = currentMillisStadyStateA;

      if (measurementsCount >= 15) { //ให้ค่าเข่าใกล้ค่าคงที่มากที่สุดก่อนเพื่อจะเริ่มจับเวลา
        STATUS_CHECK = true;
        measurementsCount = 0; // รีเซ็ต measurementsCount เมื่อครบ 15 ครั้ง ใช้เวลาเพิ่ม 15 วินาที
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