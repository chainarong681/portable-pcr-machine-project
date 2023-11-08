//PID control
#include <PID_v1.h>

//LCD 12864B v2.0
#include <U8glib.h>

//MAX31855
#include <SPI.h>
#include "Adafruit_MAX31855.h"
// digital IO pins.
#define MAXDO 9
#define MAXCS 10
#define MAXCLK 11
// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

// Peltier setup
int peltierPin = 3;  // Digital pin connected to the Peltier module

//FAN pin (PWM control)
int fanPin = 8;

// Setpoint temperatures
double rt_step = 50;                // Desired setpoint temperatures in Celsius
double initial_denature_step = 95;  // Desired setpoint temperatures in Celsius
double denature_step = 95;          // Desired setpoint temperatures in Celsius
double annealing_step = 56;         // Desired setpoint temperatures in Celsius
double extension_step = 72;         // Desired setpoint temperatures in Celsius
double cooling_step = 40; 
double warm_step = 40;          // Desired setpoint temperatures in Celsius
double currentSetpoint;


// Define variables to keep track of cycle and stage
int cycle = 0;
int stage = 0;

// Control parameters for PID
double Input, Output;
double Kp = 1;  // Proportional gain 2
double Ki = 0;  // Integral gain 5
double Kd = 0;  // Derivative gain 1
PID myPID(&Input, &Output, &currentSetpoint, 2, 5, 1, P_ON_M, DIRECT);

//LCD setting
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4, 7);  //Enable, RW, RS, RESET

void setup() {
  //Peltier
  pinMode(peltierPin, OUTPUT);  //Peltier output
  //FAN pin
  pinMode(fanPin, OUTPUT);  //FAN output
  //LCD 12864 set
  u8g.begin();  //LCD 12864 begin
  // Initialize PID control
  myPID.SetMode(AUTOMATIC);
  //MAX31855
  thermocouple.begin();
  //serial print
  Serial.begin(9600);
}

void loop() {
  //MAX31855 Read temp
  float max31855temp = thermocouple.readCelsius();  // Read temp
  if (!isnan(max31855temp)) {                       // not NaN
    Input = max31855temp - 5;                       // MAX31855 (-5 = correction value)
  }

  //Serial print every 1 sec
  unsigned long readTimeSet = 1000;
  static unsigned long readTime = 0;
  if (millis() - readTime >= readTimeSet) {
    readTime = millis();
    Serial.println(Input);
    //serial print
    //Serial.print("Temperature: ");
    //Serial.println(Input);
    //Serial.print("Cycles: ");
    //Serial.println();
    //Serial.print("Stages: ");
    //Serial.println(stage);
    //Serial.print("PID control: ");
    //Serial.println(Output);
  }

  // Print current temperature, setpoint, and PID output on the LCD
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_profont12);  // select front
    u8g.drawStr(0, 10, "C-Temp:");    //draw vertical position
    u8g.setPrintPos(55, 10);          //draw horizontal
    u8g.print(Input);
    u8g.print(" C");

    u8g.drawStr(0, 20, "Set-Temp:");  //draw vertical position
    u8g.setPrintPos(70, 20);          //draw horizontal
    u8g.print(currentSetpoint);
    u8g.print(" C");

    u8g.drawStr(0, 30, "PID Output:");  //draw vertical position
    u8g.setPrintPos(80, 30);            //draw horizontal
    u8g.print(Output);

    u8g.drawStr(0, 40, "stage:");  //draw vertical position
    u8g.setPrintPos(80, 40);       //draw horizontal
    u8g.print(stage);

    u8g.drawStr(0, 50, "cycle");  //draw vertical position
    u8g.setPrintPos(80, 50);      //draw horizontal
    u8g.print(cycle);
  } while (u8g.nextPage());
///////////////////////////////////////

RunTemp();

if (currentSetpoint == ""){
    currentSetpoint = warm_step;
    
    unsigned long TimeSet1 = 120000; //2 min
    static unsigned long readTime1 = 0;
    if (Input == warm_step && currentSetpoint == warm_step && millis() - readTime1 >= TimeSet1) {
      readTime1 = millis();
      currentSetpoint = rt_step;
    }
  } 
  unsigned long TimeSet2 = 60000; //1 sec
  static unsigned long readTime2 = 0;
  else if(Input == rt_step && currentSetpoint == rt_step && millis() - readTime2 >= TimeSet2){
    readTime2 = millis();
    currentSetpoint = initial_denature_step;
  } 
  unsigned long TimeSet3 = 60000; //1 sec
  static unsigned long readTime3 = 0;
  else if(Input == initial_denature_step &&  currentSetpoint == initial_denature_step && millis() - readTime3 >= TimeSet3){
    readTime3 = millis();
    currentSetpoint = denature_step;
  } 
  
  else if(currentSetpoint == denature_step && millis() - readTime1 >= TimeSet1){
    
  } 
  
  else if(currentSetpoint == warm_step && millis() - readTime1 >= TimeSet1){
    
  } 
  
  else if(currentSetpoint == warm_step && millis() - readTime1 >= TimeSet1){
    
  }



}

// ประกาศฟังก์ชัน
void RunTemp() {
  if (Input >= currentSetpoint) {
    myPID.Compute();
    analogWrite(peltierPin, Output); //00000
    analogWrite(fanPin, 0);
  } else {
    myPID.Compute();
    analogWrite(peltierPin, Output);
    analogWrite(fanPin, 255);
  }
}
