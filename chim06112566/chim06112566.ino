#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>
#include <U8glib.h>

// DS18B20 temperature sensor setup
#define ONE_WIRE_BUS 2 // Define the digital pin for the DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

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
double Kp = 2; // Proportional gain
double Ki = 5; // Integral gain
double Kd = 1; // Derivative gain
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

  //DS18B20
  sensors.begin(); //sensor begin
  sensors.setResolution(12); // DS18B20 set resolution 12 bit
  
  // Initialize PID control
  myPID.SetMode(AUTOMATIC);
}

void loop() {
  sensors.requestTemperatures(); //DS18B20 temp read
  float tempC = sensors.getTempCByIndex(0); //temp value show on LCD display
  //Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(tempC); //serial print
  Serial.println(totalCycles); //serial print
  
  // Compute PID control
  myPID.Compute();

  //Read temp to PID calculator
  Input = sensors.getTempCByIndex(0);
  
  Serial.println(stage); //print stage
  Serial.println(Output); //print stage

  // Print current temperature, setpoint, and PID output on the LCD
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_profont12); // select front
      u8g.drawStr(0, 10, "C-Temp:"); //draw vertical position
      u8g.setPrintPos(55, 10); //draw horizontal
      u8g.print(tempC);
      u8g.print(" C");

      u8g.drawStr(0, 30, "Set-Temp:"); //draw vertical position
      u8g.setPrintPos(70, 30); //draw horizontal
      u8g.print(Setpoint);
      u8g.print(" C");
      
      u8g.drawStr(0, 50, "PID Output:"); //draw vertical position
      u8g.setPrintPos(80, 50); //draw horizontal
      u8g.print(Output);
    } while (u8g.nextPage());

    ////////////////////////////////////////////////////////////////////////////////
  if (stage == 0) {
  // Initial denaturation
  if (Input <= denature) {
    analogWrite(peltierPin, Output); // PID output is 0-255 used to control PWM of peltier
    analogWrite(fanPin, 225); // Fan speed max 225 used to control PWM
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
  // Annealing
  if (Input <= annealing) {
    analogWrite(peltierPin, Output); // PID output is 0-255 used to control PWM of peltier
    analogWrite(fanPin, 128); // Fan speed max 225 used to control PWM
    unsigned long period2 = 10000; // ระยะเวลาที่ต้องการรอ
    static unsigned long last_time2 = 0; // ประกาศตัวแปรเป็น static เพื่อเก็บค่าไว้ไม่ให้ reset จากการวน loop
    if (millis() - last_time2 > period2) {
      last_time2 = millis(); // เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
      stage = 2; // เปลี่ยน stage เมื่อเวลาผ่านมาพอสมควร
    }
  }
}
if (stage == 2) {
  // Extension
  if (Input <= extension) {
    analogWrite(peltierPin, Output); // PID output is 0-255 used to control PWM of peltier
    analogWrite(fanPin, 225); // Fan speed max 225 used to control PWM
    if (Input >= extension) {
      unsigned long period3 = 10000; // ระยะเวลาที่ต้องการรอ
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