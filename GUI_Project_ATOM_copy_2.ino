#include "U8glib.h"
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4, 7);  //Enable, RW, RS, RESET

//loading bar
int barAdd = 0;

//MENU
#define MENU_ITEMS 4
const char *menu_strings[MENU_ITEMS] = { "PCR", "Real-time LAMP", "Real-time PCR", "Help" };
uint8_t menu_current = 0;
uint8_t menu_redraw_required = 0;

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
  delay(5000);  // rebuild the picture after some delay
  Page2();
  delay(7000);

  menu_redraw_required = 1;  //MENU
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
    //Open MENU page
    //MENU
    if (menu_redraw_required != 0) {
      u8g.firstPage();
      do {
        drawMenu();
      } while (u8g.nextPage());
      menu_redraw_required = 0;
    }

    updateMenu();  // update menu bar
  }
}

//MENU
void drawMenu(void) {
  uint8_t i, h;
  u8g_uint_t w, d;

  u8g.setFont(u8g_font_courB10);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();

  h = u8g.getFontAscent() - u8g.getFontDescent();
  w = u8g.getWidth();
  for (i = 0; i < MENU_ITEMS; i++) {
    d = (w - u8g.getStrWidth(menu_strings[i])) / 2;
    u8g.setDefaultForegroundColor();
    if (i == menu_current) {
      u8g.drawBox(0, i * h + 1, w, h);
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStr(d, i * h, menu_strings[i]);
  }
}

void updateMenu(void) {
  // if ( uiKeyCode != KEY_NONE && last_key_code == uiKeyCode ) {
  //   return;
  // }
  // last_key_code = uiKeyCode;

  // switch ( uiKeyCode ) {
  //   case KEY_NEXT:
  //     menu_current++;
  //     if ( menu_current >= MENU_ITEMS )
  //       menu_current = 0;
  //     menu_redraw_required = 1;
  //     break;
  //   case KEY_PREV:
  //     if ( menu_current == 0 )
  //       menu_current = MENU_ITEMS;
  //     menu_current--;
  //     menu_redraw_required = 1;
  //     break;
  // }
}
