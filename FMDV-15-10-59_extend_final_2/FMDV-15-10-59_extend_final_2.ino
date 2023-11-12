#include "U8glib.h"
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4, 7); // Enable, RW, RS, RESET

////////////////read A1//////////////
float value;
//loading bar
int a = 0;
//////////Set Line graph////////////
const int HEIGHT=64;
const int LENGTH=108;
int x;
int y[LENGTH];
////////////อ่านค่ากระแสจาก A1/////////
float Current;
float drawCurreunt;
//////////////หา Standardeviation อ่านค่า READ 100 ค่า////////
int i;
float s_val[108]; ////////เก็บค่า 100 ค่า
float sampleSumTotal = 0;
int SD_i; //ค่าที่จะให้รันซ้ำ
float sqDevSum = 0; ///////////หาค่า Sum จาก mean - xi/////
float stDev; ///////////////ค่า 1SD////////////////////////
float stDev3; /////////////ค่า 3SD/////////////////////////
////////////////////max and min//////////////////////////
float maxNum = 0;
float minNum = 0;
/////////////////////คำนวนค่า CT//////////////////////////
float ctMax; //ct=3sd + max
//////////////////////เซท Keypad/////////////////////////
////////////////////// set pin numbers//////////////////
const int buttonPin[] = {13,12,3,2};     //////////////// the number of the pushbutton pins
int buttonState = 0;         ///////////////// variable for reading the pushbutton status
/////////////////////จับเวลาในการอ่านค่าหา Standardeviation//////////////////
int NumTime = 0;
int num=0; ///////////////นับค่า time_count/////////////////////
int NumTime_valut = 0; /////////////////กำหนดตัวเลข progress ใหม่เป็นค่าเริ่มต้นที่ 0////////////////
//////////////////////////Menu////////////////////////////////
#define KEY_NONE 0
#define KEY_NEXT 2
#define KEY_SELECT 3
#define KEY_BACK 4
#define KEY_RUNbutton3 5
#define KEY_RUNbutton4 1
/////////////////////num 2=13/////num 1 =12/////num 4 =3/////num 3=2///////////////////////
uint8_t uiKeyRUNbutton4 = 3;
uint8_t uiKeyRUNbutton3 = 2; 
uint8_t uiKeyNext = 12; 
uint8_t uiKeySelect = 13; 
uint8_t uiKeyBack = 12; 
uint8_t uiKeyCodeFirst = KEY_NONE;
uint8_t uiKeyCodeSecond = KEY_NONE;
uint8_t uiKeyCode = KEY_NONE;
#define MENU_ITEMS 4
const char *menu_strings[MENU_ITEMS] = { "MEASURE", "HELP", "CREDIT"};
uint8_t menu_current = 0;
uint8_t menu_redraw_required = 0;
uint8_t last_key_code = KEY_NONE;
////////////////////////ค่าควบคุมปุ่มกดอ่านค่าวัดปุ่ม 3/////////////////
int Measure_Select = 0; /////////////////ปิดปุ่มรัน
int Run_measure = 0; /////////////////////////ปิดการรันเมื่อกดปุ่ม 3
///////////////////////////ค่าควบคุมปุ่มกดอ่านค่าวัดปุ่ม 4 ///////////
int button4click; /////////////////////////ปิดปุ่ม 4/////////////
/////////////////////คำนวนค่า mean จากการวัดจากปุ่ม 4/////////////
float R_val[108];
float sampleSumTotalResult;
float meanSampleResult;
int Run_measure4 = 0;
int j = 0;
float meanSample;
/////////////////////////////////////////////////////////////
void clearY(){
  for(int i=0; i<LENGTH; i++){
    y[i] = -1;
  }
}

void drawY(){
  u8g.drawPixel(0, y[0]);
  if (x <= 108){
      y[x++] = drawCurreunt;
  }
  for(int i = 9; i<LENGTH; i++){  //ให้เริ่ม plot graph ในตำแหน่งที่ 9 (X=9)
    if(y[i]!=-1 && x <= 108){
      u8g.drawLine(i-1, y[i-1], i, y[i]);  
    }
    else {
      break;
    }
    
  }
}

void Graph(){
  drawCurreunt = map(Current, 0,10 , HEIGHT-34, 15);   ////////////////ใช้ปรับค่า y////////////34 คือค่าที่เริ่มให้ plot ค่าจากแกน y
  drawY();  
}

void SampleSum(){
 ///////////Draw Currrent value////////// แสดงใน READ///////////
   u8g.setFont(u8g_font_courB08);
   char amp[3];
   dtostrf(Current,3,3,amp);
   u8g.drawStr(33,56,amp);
 //Serial.println(amp);
 ///////////////////////////หาค่า SD/////////////////////////
   if (SD_i<=108){
    s_val[SD_i] = Current;  ////////////ค่าแต่ละอันเก็บใน array///////////////
    sampleSumTotal += s_val[SD_i];  //Sum value
    meanSample = sampleSumTotal/109;
    sqDevSum += pow((meanSample - s_val[SD_i]),2);
    stDev = sqrt(sqDevSum/108);   ///////////////////หา 1sd เท่ากับ N-1////////
    stDev3 = 3*(stDev);   ////////////หา 3sd เท่ากับ N-1////////////////////
  Serial.println("AAAAAAAAAAAAAAAAAAAAAAAAA");
  Serial.print(SD_i);
  Serial.print(" value ");
  Serial.println(s_val[SD_i]);
  Serial.println(meanSample);
  Serial.println(sampleSumTotal);
  Serial.println(sqDevSum);
  Serial.println(stDev);
  /////////////////คำนวนค่า CT////////////////////
    ctMax = meanSample + stDev3; /////////////////คำนวนค่า cutoff/////////////////////
  Serial.println(ctMax);
  ///////////////หาค่า Max และ Min/////////////////
  Serial.println(maxNum);
  Serial.println(minNum);
  
////////////////////////////เปิดปุ่ม 4 ////////////////////////////////////
     if (SD_i == 108){
         button4click = 1;      
   }
  }
   SD_i++;
///////////////แสดงค่า SD//////////////////
  u8g.setFont(u8g_font_courB08);
  char std[3];
  dtostrf(stDev3,3,3,std);
  u8g.drawStr(93,56,std);
//////////////ค่า mean baseline//////////
  u8g.setFont(u8g_font_04b_24);
  u8g.drawStr(13, 6, "Mean:");
  u8g.setFont(u8g_font_04b_24);
  char meanBase[3];
  dtostrf(meanSample,3,3,meanBase);
  u8g.drawStr(32,6,meanBase);  
  
  }    

void maxMin(){
///////////////หาค่า Max และ Min/////////////////
maxNum = s_val[0];
minNum = s_val[0];
   for (i = 0; i <= 108; i++){
       if (i < 108){
         if (maxNum <= s_val[i]){
            maxNum = s_val[i];
         }
       else if (minNum >= s_val[i]){
            minNum = s_val[i];
       }
      }
   }
  //////////////////ค่า max///////////////////
   u8g.setFont(u8g_font_04b_24);
   char maxR[3];
   dtostrf(maxNum,3,3,maxR);
   u8g.drawStr(19,62,maxR);
   //////////////////ค่า min//////////////////
   char minR[3];
   dtostrf(minNum,3,3,minR);
   u8g.drawStr(54,62,minR);
}

void Result(){
///////////////////////////Baseline Show/////////////////////
     for (int a=0; a<101; a+=1){
         u8g.drawPixel(7+a,30-ctMax); //////////////////////ค่า 30 คือ ค่า 64 (height) ที่ลบออกจาก 34 ที่ให้เริ่ม plot ในแกน y
     }
///////////////////////////หาค่า mean/////////////////////////
    R_val[j] = Current;  ////////////ค่าแต่ละอันเก็บใน array///////////////
    sampleSumTotalResult += R_val[j];  //Sum value
    meanSampleResult = sampleSumTotalResult/109; //mean value
    Serial.println(j);
    Serial.println(meanSampleResult);
    Graph();

//////////////ค่า mean วัดค่าการวัด//////////
  u8g.setFont(u8g_font_04b_24);
  u8g.drawStr(50, 6, "/");
  u8g.setFont(u8g_font_04b_24);
  char mean[3];
  dtostrf(meanSampleResult,3,3,mean);
  u8g.drawStr(55,6,mean);
    
   if (j >= 107){
       if (meanSampleResult <= ctMax){
          u8g.setFont(u8g_font_courB08);
          u8g.drawStr(78, 7, "Negative");
       }  
       else if (meanSampleResult > ctMax){
          u8g.setFont(u8g_font_courB08);
          u8g.drawStr(78, 7, "Positive");
       }
    }
   Serial.println(j);     
}

void setup(void) {
     Run_measure4 = 0;
     button4click = 0;
////////////////Clear Line Graph เริ่มต้น//////////////////
     x = 0;  //เริ่มแกน x ในการพล็อตกราฟ
     clearY();
 
////////////////////เมนูเลือกเข้าเครื่อง//////////////////////
 uiSetup();    ////////// setup key detection and debounce algorithm
 menu_redraw_required = 1;     //////////// force initial redraw  
///////////////LED Pin 22 out//////////////
pinMode(22, OUTPUT);
/////////////LED Pin 24 out//////////////
pinMode(24, OUTPUT);
///////////////Keypad////////////////////
///////////////initialize the Serial Monitor @ 9600/////////////////
Serial.begin(9600);  
////////////////initialize the keypad pin(s) as an input////////////
  for(int x=0; x<4; x++){
    pinMode(buttonPin[x], INPUT_PULLUP); 
  }   
//////////////Display Rotate 180/////////////////
  u8g.setRot180();

//////////////Splash screen/////////////////////
  u8g.firstPage();  
        do {
           splasDraw(); 
        } while( u8g.nextPage() );
  delay(5000);   /////////////// rebuild the picture after some delay//////////////
           Page2();
  delay(7000);
 }

void splasDraw(void) { 
  u8g.drawFrame(0,0,128,64); //Box
  u8g.drawFrame(1,1,126,62); //Box
  u8g.setFont(u8g_font_fub20);
  u8g.drawStr( 25, 27, "FMDV");
  u8g.setFont(u8g_font_timB14);
  u8g.drawStr(4, 45, "DETECTORS");
}

void Page2(void) {
////////////////////Draw Page2//////////////////////////
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

void Box_long(uint8_t a) {
  u8g.drawBox(10,23,0+a,10); ///////////กล่องเพิ่มขนาดถึง 118
  u8g.drawFrame(10,23,108,10); //////////////กรอบนอก
  u8g.setFont(u8g_font_courB08); ////////////////ตัวอักษร
  u8g.drawStr( 31, 43, "Please wait...");
}

void draw_box(void) {   /////////////////////วาดกล่องยืด///////////////////////
 ///////////loading bar
     if (a < 108){
        Box_long(a);
          a++;
     }
}

void loop(void) {
 ///////////////////////อ่านค่าจาก A1////////////////////////////////
     value = analogRead(A1);
     Current = ((((value-511)*5)/1023)/0.0645)*20;  /////////////time 20 for amplify/////////
 //////////////////////////////////////////////////////////////////
 //////////////Draw loading bar
          if (a < 108){
             u8g.firstPage();
                do  {
                     draw_box();
                    } while( u8g.nextPage() );
                delay(30);
          }
          if (a >= 108){
              Menu_select();
          }                    
}

void LabelMeasure(){
   ////////////กรอบกราฟ
         u8g.drawLine(7, 0, 7, 45); //บาร์ตั้ง
         u8g.drawLine(7, 45, 107, 45); //บาร์นอน
         //u8g.drawLine(50, 8, 50, 64);
         //u8g.drawLine(0, 7, 50, 7);
  ///////////ตั้งเลเบลแกน x และ Y
         u8g.setFont(u8g_font_micro);
         u8g.drawStr(110, 46, "time");
         u8g.drawStr270(5, 43, "Current(mA)");
  ////////////วาดสเกลบนกรอบของกราฟแนวตั้ง
     for (int x=0; x<45; x+=5){
         u8g.drawPixel(6,0+x);
   ///////////วาดสเกลบนกรอบของกราฟแนวนอน
     for (int x=0; x<102; x+=5){
         u8g.drawPixel(7+x,46);
     }
     ///////////วาดสเกล 0////////
     for (int x=0; x<102; x += 5){
         u8g.drawPixel(7+x,30);
     }
     } 
  //////////////Label หน้าวัดค่า////////////////////
         u8g.drawFrame(0,47,128,17); //Box
         u8g.setFont(u8g_font_courB08);
         u8g.drawStr(3, 56, "Read:");
         u8g.setFont(u8g_font_04b_24);
         u8g.drawStr(3, 62, "Max:");
         u8g.drawStr(41, 62, "Min:");
         u8g.setFont(u8g_font_courB08);
         u8g.drawStr(70, 56, "3SD:");
         u8g.setFont(u8g_font_04b_24);
         u8g.drawStr(111, 33, "0(mV)");
}

void Time_Count(){
       NumTime = 0.92*(num);
       if (NumTime <= 99){
           u8g.setFont(u8g_font_04b_24);
           u8g.setPrintPos(111, 62);
           u8g.print(NumTime);
           u8g.drawStr(77, 62, "Progress"); 
           u8g.drawStr(120, 62, "%"); 
      }
      if (NumTime >= 100){
           NumTime_valut = 1;
      }
       num++;
}

///////////////////////////เมนูเลือกด้วย Library//////////////////////////////////////////////
void uiSetup(void) {
  // configure input keys 
  pinMode(uiKeyNext, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeySelect, INPUT_PULLUP);           // set pin to input with pullup
  pinMode(uiKeyBack, INPUT_PULLUP);           // set pin to input with pullup
}

void uiStep(void) {
  uiKeyCodeSecond = uiKeyCodeFirst;
  if ( digitalRead(uiKeyNext) == LOW ) /////////ปุ่ม1
    uiKeyCodeFirst = KEY_NEXT;
  else if ( digitalRead(uiKeySelect) == LOW ) //////////ปุ่ม2
    uiKeyCodeFirst = KEY_SELECT;
  else if ( digitalRead(uiKeyBack) == LOW )////////////ปุ่ม1
    uiKeyCodeFirst = KEY_BACK;
  else 
    uiKeyCodeFirst = KEY_NONE;
  if ( uiKeyCodeSecond == uiKeyCodeFirst )
    uiKeyCode = uiKeyCodeFirst;
  else
    uiKeyCode = KEY_NONE;
}

void drawMenu(void) {
  uint8_t i, h;
  u8g_uint_t w, d;
  u8g.setFont(u8g_font_helvB12);
  u8g.setFontPosTop();
  h = u8g.getFontAscent()-u8g.getFontDescent();
  w = u8g.getWidth();
  for( i = 0; i < MENU_ITEMS; i++ ) {  //////////จำนวนบาร์ที่เลื่อนได้ในเมนู
    d = (w-u8g.getStrWidth(menu_strings[i]))/2;  ///////////การหาร 2 เพื่ให้ข้อความอยู่ตรงกลางจอ
    u8g.setDefaultForegroundColor();
    if ( i == menu_current ) {
      u8g.drawBox(0, i*h+8, w, h-1);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i*h+8, menu_strings[i]); //////////////ตำแหน่งไอคอนตรงกลาง u8g.drawStr(d, i*h+18, menu_strings[i]); ////////////////ตำแหน่งไอคอน
  }
}

void updateMenu(void) {
  if ( uiKeyCode != KEY_NONE && last_key_code == uiKeyCode ) {
     return;
  }
  last_key_code = uiKeyCode;
  switch ( uiKeyCode ) {
    case KEY_NEXT:
        if (menu_current == 0 && Measure_Select > 1 && Run_measure == 1) //ป้องกันไม่ให้เกิด error ในการกดปุ่ม 1 ในขณะรันหลังจากกดปุ่ม 3 แล้ว
             break;
             menu_current++;
        if ( menu_current >= 3 ) //เลือกเมนูได้ 3 จาก 4 เมนู
             menu_current = 0;
             menu_redraw_required = 1;
             Measure_Select = 0; //ส่งค่า 0
             break;
       
    case KEY_SELECT:
        if ( menu_current == 0 ){ 
             Measure_Select = 1; //ส่งค่าเป็น 1 ให้ปุ่ม 3 ทำงานได้
           if (Run_measure >= 0){
                   Run_measure = 0;
                   clearY(); ////////////////////เคลียกราฟ
                   y[LENGTH]=-1; ////////////////เคลีย array
                   x = 0;
                   updateMenu();
                   SD_i = 0; ///////////////////ล้างค่านับจำนวนเพื่อใช้คำนวน SD ใหม่
                   NumTime_valut = 0; /////////////////////กำหนดตัวเลข progress ใหม่เป็นค่าเริ่มต้นที่ 0
                   num = 0;      
                   s_val[108]= -1;
                   sampleSumTotal = 0;
                   sqDevSum = 0;
                   stDev = 0; 
                   stDev3 = 0; 
                   meanSample = 0;
                   maxNum = 0;
                   minNum = 0;
                   ctMax = 0;
                   j = 0;
                   Run_measure4 = 0;
                   R_val[108] = -1;
                   sampleSumTotalResult = 0;
                   meanSampleResult = 0;
                   button4click = 0;  
           }
             u8g.firstPage();
               do  {
                   LabelMeasure();
               } while( u8g.nextPage() );
             break;
        }
        
       else if ( menu_current == 1 ){
             page_HELP();
             break;
       }
       
       else if ( menu_current == 2 ){
             page_Credit();
       break;
       }
  }
}

//////////////////////////////////////Run หน้า menu/////////////////////////////////////////
void Menu_select(){
 uiStep(); //////////////// check for key press
    
    if (  menu_redraw_required != 0 ) {
          u8g.firstPage();
       do  {
          drawMenu();
       } while( u8g.nextPage() );
          menu_redraw_required = 0;
    }

     updateMenu(); ////////////////////update menu bar        

/////////////////Keypad ควบคุมปุ่ม 3 และ 4////////////////////
     for(int x=0; x<4; x++){
     buttonState = digitalRead(buttonPin[x]);
        
          if (menu_current == 0 && Measure_Select == 1 && buttonState == LOW && buttonPin[x] == 2) {
             ////////////ปุ่ม 3
             digitalWrite(22, HIGH);
             delay(1000);
             digitalWrite(22, LOW);
             delay(1);
             Run_measure = 1; //ส่งค่าเป็น 1
             Measure_Select = 2;
             button4click = 0;
             
          }
    
         if (button4click == 1 && buttonState == LOW && buttonPin[x] == 3) {
            /////////////ปุ่ม 4
            digitalWrite(24, HIGH);
            delay(1000);
            digitalWrite(24, LOW);
            delay(1);
            Run_measure4 = 1;
            Measure_Select = 2;
            button4click = 0;     
         } 
     }

/////////////////////////////Run button control 3 และ 4////////////////////////  
         if (Run_measure == 1){
            Run();
         } 
}

/////////////////////////////หน้าช่วยเหลือ///////////////////////////////////////
 void page_HELP(){
          u8g.firstPage();
             do  {
                 u8g.setFont(u8g_font_courB08);
                 u8g.drawStr(5, 10, "User manual");
            } while( u8g.nextPage() );
 }
 
/////////////////////////////หน้า credit//////////////////////////////////////
 void page_Credit(){
          u8g.firstPage();
              do  {
                u8g.drawFrame(0,0,128,64);
                u8g.setFont(u8g_font_ncenB12);
                u8g.drawStr( 20, 19, "Develop by"); 
                u8g.setFont(u8g_font_7x13B);
                u8g.drawStr( 2, 33, "Chainarong.Kulchim");
                u8g.drawStr( 4, 44, "Medical scientist");
                u8g.drawStr( 7, 55, "Tel: 080-5040716");
                u8g.setFont(u8g_font_micro);
                u8g.drawStr( 22, 62, "Chainarong681@gmail.com");
              } while( u8g.nextPage() );
 }

void Run(){
          u8g.firstPage();
               do  {
              if (Run_measure4 == 1){  /////////ปุ่ม 4 ทำงาน/////////////
                   if (j <= 108){
                       Result();
                   }
                   if (j > 108){
                       break;
                   }
                   j++;
             }
                   LabelMeasure();
                   Time_Count(); //////////////////////จับเวลา 60 วินาที
                   SampleSum();
                   maxMin();
                   
///////////////////////// ถ้าจับเวลาครบให้แจ้ง OK//////////////////////
               if (NumTime_valut == 1){
                  u8g.setFont(u8g_font_04b_24);
                  u8g.drawStr(77, 62, "Progress");
                  u8g.drawStr(112, 62, "OK");
               }               
               } while( u8g.nextPage() );
        delay(60);
}

