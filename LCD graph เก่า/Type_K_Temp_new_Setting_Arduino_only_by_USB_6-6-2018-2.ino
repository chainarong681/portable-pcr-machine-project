//Display library
#include "U8glib.h"
U8GLIB_ST7920_128X64_1X u8g(6,5,4,7); //Enable, RW, RS, RESET

//Read temp
float temp_value;
float temp_value_graph;
int numTime;

//Set Graph
const int WIDTH=86; // Set 14 to 86 = 72 point
const int HEIGHT=64;
const int LENGTH=WIDTH;
int x;
int y[LENGTH];

//Average 3-days Data
const int numReadings = 72;    //72 Value ไม่ได้ใช้หลัก array ใช้การลบออกในสูตร
float readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

//max and min
float maxNum =0;
float minNum =0;

//Data logger
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
RTC_DS1307 rtc;
File dataFile; //SD card
const int chipSelect = 10; //Pin 10 SD card

//Store to variable from SD card Data
int CS_PIN = 10;
File file;
int recNum = 0; // We have read 0 records so far
//Setting for Arduino MEGA
String freezer_setting_temp;
float freezer_setting_tempFloat; //Freezer tolerance
String freezer_Time_Warnning;
float freezer_Time_WarnningFloat;// Time warnning setting
String correctionFactor;
float correctionFactorFloat;// correctionFactor setting

//LED blink for read Temp
int ledPin = 40;
 
//Library for MAX31855
#include "Adafruit_MAX31855.h"

// Default connection is using software SPI, but comment and uncomment one of
// the two examples below to switch between software SPI and hardware SPI:
// Example creating a thermocouple instance with software SPI on any three
// digital IO pins.
#define MAXDO   14
#define MAXCS   15
#define MAXCLK  16

// initialize the Thermocouple
Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);

void setup() {
// start serial port
  Serial.begin(9600);//Send by TX RX to ESP8266-01. IT match to serial buadrate(ESP8266-01).
//If you want to send temp data to esp8266-01.you must comment serial.println to other data funtion.but you must use serial.println ===>Temp only.
  
// wait for MAX31855 chip to stabilize
  delay(500);

u8g.setRot180(); // Rotate 180 Displat

// Draw Splash screen tempalarm
  u8g.firstPage();
      do  {
            splash_TempAlarm();
          }while(u8g.nextPage());
      delay (5000); // rebuild the picture after some delay
            Page_credit();
      delay(5000);   
            main_page();

//Set Graph
       x = 14; //Reset at 10 point
       clearY();

//RTC ===>Real time clock setting
    rtc.begin();
    if (! rtc.isrunning()) {
    //Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }

//Data logger setting
    // Open serial communications and wait for port to open:
    //Serial.begin(9600);
        while (!Serial) {
              ; // wait for serial port to connect. Needed for Leonardo only
        }
    //Serial.print("Initializing SD card...");
    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(SS, OUTPUT);
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        //Serial.println("Card failed, or not present");
    // don't do anything more:
        while (1) ;
    }
    //Serial.println("card initialized.");
    // Open up the file we're going to log to!
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (! dataFile) {
        //Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
        while (1) ;
    }
    
    // LED blink for Read Temp
    pinMode(ledPin, OUTPUT);

//SD reading to variable/////////////////////////////////////////////
  pinMode(CS_PIN, OUTPUT);
  SD.begin();
  file = SD.open("setting.txt");
  while (file.available())
 {
    String list = file.readStringUntil('\n');
    //Serial.println(list);
    if (recNum <=4){
      recNum++;  // Count the record
    }
//    recNum++;  // Count the record
    if(recNum == 1){
       freezer_setting_temp = list; //Store Sdcard value to variable
    }
    if(recNum == 2){
       freezer_Time_Warnning = list; // Store sdcard value to variable
    }
    if(recNum == 3){
       correctionFactor = list; // Store sdcard value to variable
    }
 }
}

void loop() {
  //Code for MAX31855
  //Time count for Loop: //Best code for replacemnt of delay frunction
  static unsigned long Time_sensors = millis();
  if ((millis() - Time_sensors) > 5000) { //delay time every 5 Sec(5000).
      Time_sensors = millis();

    //MAX31855
    // basic readout test, just print the current temp
    //Serial.print("Internal Temp = ");
    //Serial.println(thermocouple.readInternal());
        double c = thermocouple.readCelsius();

    // If isnan (isnan is "is not a number") prtect from nois or electronic interfere for environment......
    //Read Temperature from moduleMAX31855
        if (isnan(c)){
           //Serial.println("Something wrong with thermocouple!");
           // do not send Temperature to ARDUINO....use empty function
        } else {
           //Serial.print("C = "); 
           //Serial.println(c);
           //Serial.print("Temperature for Device 1 is: ");
           correctionFactorFloat = correctionFactor.toFloat();
           temp_value = c + correctionFactorFloat; //Temp + correction factor
           Serial.println(temp_value); // Why "byIndex"?======>send data to esp8266-01 by serial TX and RX
        }
           //Serial.print("F = ");
           //Serial.println(thermocouple.readFarenheit());
     
     //LED Blink
     digitalWrite(ledPin, HIGH); //Open LED every 10 sec at pin = 36
     delay(1000);
     digitalWrite(ledPin, LOW);
   }

  // Draw Graph 
   u8g.firstPage();  
        do {
          drawY();
          Label_Graph();
          temp_show();
          avg_max_min();
      } while( u8g.nextPage() );


  //Time count for average: //Best code for replacemnt of delay frunction
  freezer_setting_tempFloat = freezer_setting_temp.toFloat(); //Convert String to float about Freezer tolerence
  
  static unsigned long Time = millis();
         if ((millis() - Time) > 3600000 && temp_value <= freezer_setting_tempFloat) { //delay time every 1 hrs(3600000).
               Time = millis();
               temp_3dayData();//3-days Data 
               writeDataToSD();//Write to SD card
         //Graph draw line
                    x++;
                    if(x >= WIDTH){
                          x = 14; //Reset at 14 point
                          clearY();
                    }
          }

  //Warning temperature limit. Write to SD card
  freezer_Time_WarnningFloat = freezer_Time_Warnning.toFloat(); //Convert String to float about Freezer tolerence
  static unsigned long Time_writeError = millis();
          if (temp_value >= freezer_setting_tempFloat && (millis() - Time_writeError) > freezer_Time_WarnningFloat){ //Every Time setting.
              Time_writeError = millis();
              writeData_warning_ToSD();
          } 
}

void splash_TempAlarm(){
  u8g.drawFrame(0,0,128,64); // Draw Box
  u8g.drawFrame(1,1,126,62); // Draw Box
  u8g.setFont(u8g_font_fub20);
  u8g.drawStr( 5, 30, "Temp");
  u8g.setFont(u8g_font_timB14);
  u8g.drawStr(70, 45, "Alarm");
}

void Page_credit() {
//Draw Page2
u8g.firstPage();
      do  {
           u8g.setFont(u8g_font_ncenB12);
           u8g.drawStr( 20, 19, "Develop by"); 
           u8g.setFont(u8g_font_7x13B);
           u8g.drawStr( 1, 33, "Chainarong Kulchim");
           u8g.setFont(u8g_font_micro);
           u8g.drawStr( 22, 42, "Chainarong681@gmail.com");
       } while( u8g.nextPage() );
}

void main_page() {
    if (numTime < 100){
        u8g.firstPage();
         do  {
               u8g.setFont(u8g_font_osb35);
               u8g.drawStr( 30, 50, "GO");
             } while( u8g.nextPage() );
    }
  numTime++;
  delay(3000);  
}

void clearY(){
  for(int i=0; i<LENGTH; i++){
    y[i] = -1;
  }
}

void drawY(){
  temp_value_graph = temp_value;
  y[x] = map(temp_value_graph, 0, 1023, HEIGHT-32, -255); // value -32 is adjust position, -255 convert resolution from 1023
  
    
  u8g.drawPixel(14, y[0]);
  for(int i=14; i<LENGTH; i++){
    if(y[i]!=14){
      u8g.drawLine(i, y[i], i, y[i]); // Line, Start at 10 dot
    }else{
      break;
    }
  }
}

//Label Graph and Label parameter
void Label_Graph(){
   //กรอบกราฟ
         u8g.drawLine(14, 0, 14, 64);// Left vertical
         u8g.drawLine(14, 32, 86, 32); // central 0'C
         u8g.drawFrame(0,0,89,64); //Box Graph
  /* //ตั้งเลเบลแกน x และ Y
         u8g.setFont(u8g_font_micro);
         u8g.drawStr(10, 6, "Time(sec)");
         u8g.drawStr270(58, 56, "Current(mA)"); */
  //วาดสเกลบนกรอบของกราฟแนวตั้ง
     for (int x=0; x<64; x+=8){
         u8g.drawPixel(13,8+x);
         u8g.drawPixel(12,8+x);
   //วาดสเกลบนกรอบของกราฟแนวตั้ง Day-1
          for (int x=0; x<64; x+=2){
              u8g.drawPixel(39,0+x);
          }
   //วาดสเกลบนกรอบของกราฟแนวตั้ง Day-2
          for (int x=0; x<64; x+=2){
              u8g.drawPixel(63,0+x);
          }
   //วาดสเกลบนกรอบของกราฟแนวตั้ง Day-3
          for (int x=0; x<64; x+=2){
              u8g.drawPixel(87,0+x);
          }             
  
     } 
  //Label หน้าวัดค่า////////////////////
         u8g.drawFrame(88,0,40,64); //Box
         u8g.setFont(u8g_font_u8glib_4);
         u8g.drawStr(4, 10, "90");
         u8g.drawStr(4, 18, "60");
         u8g.drawStr(4, 26, "30");
         u8g.drawStr(8, 34, "0");
         u8g.drawStr(0, 42, "-30");
         u8g.drawStr(0, 50, "-60");
         u8g.drawStr(0, 58, "-90");
         u8g.drawStr(19, 61, "Day 1");
         u8g.drawStr(43, 61, "Day 2");
         u8g.drawStr(67, 61, "Day 3");
         u8g.drawStr(90, 24, "3Day AVG:");
         u8g.drawStr(90, 38, "3Day MAX:");
         u8g.drawStr(90, 52, "3Day MIN:");
}

void temp_show(){
  //Temp value
   u8g.setFont(u8g_font_helvB08);
   char value[2]; //Decimal show 2 point
   dtostrf(temp_value,2,2,value);
   u8g.drawStr(95,17,value);
   u8g.setFont(u8g_font_4x6);
   u8g.drawStr(90, 7, "Temp:");  
}

void temp_3dayData(){
        //Average
        // read from the sensor:
            readings[readIndex] = temp_value;
        // add the reading to the total:
            total = total + readings[readIndex];
        // advance to the next position in the array:
            readIndex = readIndex + 1;

        // if we're at the end of the array...
            if (readIndex >= numReadings) {
        // ...wrap around to the beginning:
            readIndex = 0;
            total = 0;
            }
        // calculate the average:
            average = total / readIndex;
        // send it to the computer as ASCII digits
        
        //MAx and Min
          maxNum = readings[0]; 
          minNum = readings[0];
          for (int i = 0; i <= 71; i++){ //คิดแบบ array 0-71=72, เริ่มคิดที่ array =0
              if (i < 71){
                if (maxNum <= readings[i]){
                    maxNum = readings[i];
                }
                else if (minNum >= readings[i]){
                   minNum = readings[i];
                }
              }
          }
}

void avg_max_min(){
  //Average Temp
   u8g.setFont(u8g_font_micro);
   char value[2]; //Decimal show 2 point
   dtostrf(average,2,2,value);
   u8g.drawStr(95,32,value);
   //Max Temp 
   char value2[2]; //Decimal show 2 point
   dtostrf(maxNum,2,2,value2);
   u8g.drawStr(95,46,value2);
   //Min Temp 
   char value3[2]; //Decimal show 2 point
   dtostrf(minNum,2,2,value3);
   u8g.drawStr(95,60,value3);
}

void writeDataToSD(){
   //White Data to SD caed
//Time Sample ====> 2018/3/25 12:19:21
DateTime now = rtc.now();
//    Serial.print(now.year(), DEC);
//    Serial.print('/');
//    Serial.print(now.month(), DEC);
//    Serial.print('/');
//    Serial.print(now.day(), DEC);
//    Serial.print(' ');
//    Serial.print(now.hour(), DEC);
//    Serial.print(':');
//    Serial.print(now.minute(), DEC);
//    Serial.print(':');
//    Serial.print(now.second(), DEC);
//    Serial.println();
    
// make a string for assembling the data to log:
 String dataString = "";
 // read three sensors and append to the string:
dataString = String(now.year(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.day(), DEC) + "  Time=" + String(now.hour(), DEC) 
              + ":" + String(now.minute(), DEC) + ":" +String(now.second(), DEC) +"  Current Temp.=" + String (temp_value) + "'C  Average Temp.=" + String(average) + "'C  Max Temp.="
              + String(maxNum) + "'C  Min Temp.=" + String(minNum) + "'C";
dataFile.println(dataString);
// print to the serial port too:
//Serial.println(dataString);
dataFile.flush();
}

void writeData_warning_ToSD(){
   //White Data to SD caed
//Time Sample ====> 2018/3/25 12:19:21
DateTime now = rtc.now();
//    Serial.print(now.year(), DEC);
//    Serial.print('/');
//    Serial.print(now.month(), DEC);
//    Serial.print('/');
//    Serial.print(now.day(), DEC);
//    Serial.print(' ');
//    Serial.print(now.hour(), DEC);
//    Serial.print(':');
//    Serial.print(now.minute(), DEC);
//    Serial.print(':');
//    Serial.print(now.second(), DEC);
//    Serial.println();
    
// make a string for assembling the data to log:
 String dataString = "";
 // read three sensors and append to the string:
dataString = "WARNING!!!!! Temperature condition. The temperature is higher than " + String(freezer_setting_tempFloat) + " degrees (" + String(now.year(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.day(), DEC) + "===>" + String(now.hour(), DEC) 
              + ":" + String(now.minute(), DEC) + ":" +String(now.second(), DEC) +"  ,Now current Temp. is " + String (temp_value) + "'C  ,Average Temp.= " + String(average) + "'C  ,Max Temp.= "
              + String(maxNum) + "'C  and Min Temp.=" + String(minNum) + "'C";
dataFile.println(dataString);
// print to the serial port too:
//Serial.println(dataString);
dataFile.flush();  
} 


