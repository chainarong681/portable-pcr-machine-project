#include "U8glib.h"
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4, 7);  //Enable, RW, RS, RESET

//loading bar
int barAdd = 0;

void splasDraw(void) {           //Splash
  u8g.drawFrame(0, 0, 128, 64);  //Box
  u8g.drawFrame(1, 1, 126, 62);  //Box
  u8g.setFont(u8g_font_fub20);
  u8g.drawStr(18, 27, "Project");
  u8g.setFont(u8g_font_timB14);
  u8g.drawStr(30, 50, "ATOM");
}

void Page2(void) {  //Page 2
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_ncenB12);
    u8g.drawStr(20, 19, "Develop by");
    u8g.setFont(u8g_font_7x13B);
    u8g.drawStr(1, 33, "Chainarong Kulchim");
    u8g.setFont(u8g_font_micro);
    u8g.drawStr(22, 42, "Chainarong681@gmail.com");
  } while (u8g.nextPage());
}

void Box_long(uint8_t a) {
  u8g.drawBox(10, 23, 0 + a, 10);  //draw box valuemax = 118
  u8g.drawFrame(10, 23, 108, 10);  //Outer frame
  u8g.setFont(u8g_font_courB08);   //Front
  u8g.drawStr(31, 43, "Please wait...");
}

void draw_box(void) {
  //Box extend
  //loading bar
  if (barAdd < 108) {
    Box_long(barAdd);
    barAdd++;
  }
}

void setup() {
  //Splash screen
  u8g.firstPage();
  do {
    splasDraw();
  } while (u8g.nextPage());
  delay(5000); // rebuild the picture after some delay
  Page2();
  delay(7000);
}

void loop() {
  //Draw loading bar
  if (barAdd < 108) {
    u8g.firstPage();
    do {
      draw_box();
    } while (u8g.nextPage());
    delay(30);
  }
  if (barAdd >= 108) {
    //Menu_select(); ใส่หน้า 2 ที่จะเปิด
  }
}
