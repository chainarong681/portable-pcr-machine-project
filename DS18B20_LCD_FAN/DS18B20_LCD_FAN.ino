#include <U8glib.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// กำหนดขาที่ใช้สำหรับ DS18B20
const int oneWireBus = 2;

int fanPin = 8;  // กำหนดขาที่เชื่อมกับสายสัญญาณ PWM ของพัดลม

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
  //sensors.setResolution(12); // ตั้งค่าความละเอียดเป็น 12 บิต
  
   pinMode(fanPin, OUTPUT);  // ตั้งค่าขาเป็นขาส่งออก
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
      
    } while (u8g.nextPage());
    
    // ตรวจสอบอุณหภูมิและควบคุมพัดลม
    if (temperatureC > 35.0) {
      // ควบคุมพัดลมด้วย PWM เท่ากับ 225
      analogWrite(fanPin, 225);
    } else {
      // ปิดพัดลม
      analogWrite(fanPin, 0);
    }
  } else {
    // หากไม่สามารถอ่านค่าได้ ให้แสดงข้อความผิดพลาด
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_6x10);
      u8g.drawStr(20, 35, "Error reading");
    } while (u8g.nextPage());
    
    // ปิดพัดลมหากมีข้อผิดพลาด
    analogWrite(fanPin, 0);
  }
  
  delay(1000);  // รอ 2 วินาที
}
