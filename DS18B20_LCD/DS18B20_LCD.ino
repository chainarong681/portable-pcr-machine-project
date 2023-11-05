#include <U8glib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// กำหนดขาที่ใช้สำหรับ DS18B20
const int oneWireBus = 2;

// สร้าง OneWire instance
OneWire oneWire(oneWireBus);

// สร้าง DallasTemperature instance
DallasTemperature sensors(&oneWire);

// กำหนดขนาดของจอ OLED (ให้ตรงกับของคุณ)
U8GLIB_ST7920_128X64_1X u8g(6, 5, 4 ,7); //Enable, RW, RS, RESET 

void setup(void) {
  // กำหนดความละเอียดของจอ OLED
  u8g.setColorIndex(1);  // 1 คือสีขาว

  // สร้างการเริ่มต้นสำหรับ DS18B20
  sensors.begin();
}

void loop(void) {
  // อ่านค่าอุณหภูมิจาก DS18B20
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  // ตรวจสอบว่าอุณหภูมิถูกอ่านได้หรือไม่
  if (temperatureC != DEVICE_DISCONNECTED_C) {
    // แสดงค่าอุณหภูมิบนจอ OLED
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(0, 15, "Temperature:");
      u8g.setPrintPos(60, 35);
      u8g.print(temperatureC, 1);  // แสดงค่าอุณหภูมิที่มี 1 ตำแหน่งทศนิยม
      u8g.drawStr(80, 35, "\xb0C");  // องศาเซลเซียส
    } while (u8g.nextPage());
  } else {
    // หากไม่สามารถอ่านค่าได้ ให้แสดงข้อความผิดพลาด
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(20, 35, "Error reading");
    } while (u8g.nextPage());
  }
  
  delay(2000);  // รอ 2 วินาที
}
