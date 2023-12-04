///////////////////////////ส่วน WIFI////////////////////////////////////////
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>

const char *ssid = "Project_nano";
const char *password = "123456789";

IPAddress apIP(192, 168, 1, 5);
IPAddress gatewayIP(192, 168, 1, 1);
IPAddress subnetIP(255, 255, 255, 0);

WebServer server(80);

///////////////////////////ส่วนแสดง LCD/////////////////////////////////////
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/18, /* data=*/23, /* CS=*/5, /* reset=*/22);  // ESP32

//loading bar
int boxlong = 0;

//Read temp
float READTEMP;

//อ่าน EEPROM เก็บเพื่ใช้เป็นค่า setting
float methodRUN;
int timeRUN;
float temCRUN;

//สถานะการทำงาน
int State_Run = 0;

//ประกาศตัวแปรเป็น global เพื่อเก็บค่าไว้ไม่ให้ reset จากการวนloop
unsigned long last_time = 0;
unsigned long period = 500;  //ระยะเวลาที่ต้องการรอ 1 นาที (1000 * 60)
int TIME_COUNT = 0;

//Set Line graph
float temperature[90];  // Array to store temperature values
int currentIndex;   // Index to keep track of the current position in the array
int igraph;


/////////////////////////////////WIFI//////////////////////////////////////
// Function to write a string to EEPROM
void writeStringToEEPROM(int addr, const String &str) {
  for (size_t i = 0; i < str.length(); i++) {
    EEPROM.write(addr + i, str[i]);
  }
  EEPROM.write(addr + str.length(), '\0');  // Null-terminate the string
  EEPROM.commit();                          // Commit changes to EEPROM
}

// Function to read a string from EEPROM
String readStringFromEEPROM(int addr) {
  String result = "";
  char ch = EEPROM.read(addr);
  while (ch != '\0') {
    result += ch;
    addr++;
    ch = EEPROM.read(addr);
  }
  return result;
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1 style=\"color:blue;\">Project nano</h1>";
  html += "<h2 style=\"color:black;\">This project was developed by Mr. Chainarong Kulchim (medical scientist) </h2>";
  html += "<h2 style=\"color:black;\">Veterinary Research and Development Center Lower Northern Region</h2>";
  html += "<h2 style=\"color:black;\">9 Moo.15 Phitsanulok-Lomsak road Wangthong Phitsanulok 65130</h2>";
  html += "<h2 style=\"color:black;\">Email: chainarong681@gmail.com Tel.0805040716</h2>";

  // เพิ่มขนาดและเปลี่ยนสีปุ่ม
  html += "<button style=\"font-size: 30px; background-color: lightgreen;\" onclick=\"goToSecondPage()\">Run program</button>  ";
  html += "<button style=\"font-size: 30px; background-color: lightyellow;\" onclick=\"goToThirdPage()\">Setting program</button>  ";
  html += "<button style=\"font-size: 30px; background-color: lightpink;\" onclick=\"goToFourthPage()\">Calibration</button>";

  html += "<script>function goToSecondPage() { window.location.href = '/second'; }</script>";
  html += "<script>function goToThirdPage() { window.location.href = '/third'; }</script>";
  html += "<script>function goToFourthPage() { window.location.href = '/Fourth'; }</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleSecondPage() {
  String html = "<html><body>";
  html += "<button style='font-size: 30px; background-color: lightgreen;' onclick='goToRoot()'>Back</button>";
  html += "<h1>Project NANO Run:</h1>";

  // Read values from EEPROM
  String method = readStringFromEEPROM(0);
  String time = readStringFromEEPROM(5);
  String tempC = readStringFromEEPROM(10);

  html += "<p style='font-size: 28px;'>Run method: " + method + "</p>";
  html += "<p style='font-size: 28px;'>Run time: " + time + "</p>";
  html += "<p style='font-size: 28px;'>Temp Correction: " + tempC + "</p>";

  // เพิ่มปุ่ม Start Run และ Stop Run
  html += "<button style='font-size: 24px; background-color: lightblue;' onclick='startRun()'>Start Run</button> ";
  html += "<button style='font-size: 24px; background-color: lightcoral;' onclick='stopRun()'>Stop Run</button> ";
  html += "<button style='font-size: 24px; background-color: lightyellow;' onclick='resetRun()'>Reset Run</button> ";
  html += "<script>";
  html += "function startRun() {";
  html += "  if (confirm('Are you sure to start RUN?')) {";
  html += "    fetch('/startRun', { method: 'POST' })";
  html += "      .then(response => response.json())";
  html += "      .then(data => {";
  html += "        console.log('State_Run updated:', data.State_Run);";
  html += "      });";
  html += "  }";
  html += "}";
  html += "function stopRun() {";
  html += "  if (confirm('Are you sure you want to stop?')) {";
  html += "    fetch('/stopRun', { method: 'POST' })";
  html += "      .then(response => response.json())";
  html += "      .then(data => {";
  html += "        console.log('State_Run updated:', data.State_Run);";
  html += "      });";
  html += "  }";
  html += "}";
    html += "function resetRun() {";
  html += "  if (confirm('Are you sure you want to reset Run?')) {";
  html += "    fetch('/resetRun', { method: 'POST' })";
  html += "      .then(response => response.json())";
  html += "      .then(data => {";
  html += "        console.log('State_Run updated:', data.State_Run);";
  html += "      });";
  html += "  }";
  html += "}";
  html += "</script>";
  html += "<script>function goToRoot() { window.location.href = '/'; }</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleThirdPage() {
  String html = "<html><body>";
  html += "<button style='font-size: 30px; background-color: lightgreen;' onclick='goToRoot()'>Back</button>";
  html += "<h1>Amplification method setting:</h1>";
  html += "<form action='/submit' method='post'>";
  html += "<label for='methods' style='font-size: 24px;'>Choose a method: </label>";
  html += "<select name='methods' id='methods' style='font-size: 24px;'>";
  html += "<option value='37'>Recombinase Polymerase Amplification(37 'C)</option>";
  html += "<option value='42'>Recombinase Polymerase Amplification(42 'C)</option>";
  html += "<option value='60'>Loop-mediated isothermal amplification(60 'C)</option>";
  html += "<option value='65'>Loop-mediated isothermal amplification(65 'C)</option>";
  html += "</select><br><br>";
  html += "<label for='time' style='font-size: 24px;'>Choose a time: </label>";
  html += "<select name='time' id='time' style='font-size: 24px;'>";
  html += "<option value='30'>30 min</option>";
  html += "<option value='60'>60 min</option>";
  html += "<option value='90'>90 min</option>";
  html += "</select><br><br>";
  html += "<input type='submit' value='Submit' style='font-size: 20px; width: 100px; height: 30px; background-color: yellow;'>";
  html += "</form>";
  html += "<script>function goToRoot() { window.location.href = '/'; }</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleThirdPageSubmit() {
  String method = server.arg("methods");
  String time = server.arg("time");

  // Assuming you have enough space in EEPROM for the method and time strings
  writeStringToEEPROM(0, method);
  writeStringToEEPROM(5, time);

  // Print the saved values to Serial
  Serial.println("Method: " + method);
  Serial.println("Time: " + time);

  // Assuming you have some condition to determine success or failure
  bool success = true;  // Change this based on your actual condition

  String message;
  String color;

  if (success) {
    message = "Data saved successfully";
    color = "green";
  } else {
    message = "Failed to save data";
    color = "red";
  }

  String html = "<html><body>";
  html += "<h1 style='color:" + color + "'>" + message + "</h1>";
  html += "<button style='font-size: 30px; background-color: lightgreen;' onclick='goToThirdPage()'>Back</button>";
  html += "<script>function goToThirdPage() { window.location.href = '/third'; }</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleFourthPage() {
  String html = "<html><body>";
  html += "<button style='font-size: 30px; background-color: lightgreen;' onclick='goToRoot()'>Back</button>";
  html += "<h1 style='font-size: 24px;'>Temperature calibration:</h1>";
  // เพิ่มฟอร์ม input ข้อมูล
  html += "<form action='/submitFourth' method='post' onsubmit='return validateForm()'>";
  html += "<label for='tempC' style='font-size: 24px;'>Temp correction ('C): </label>";
  html += "<input type='text' id='tempC' name='tempC' style='font-size: 24px;' required><br><br>";
  html += "<input type='submit' value='Submit' style='font-size: 20px; width: 100px; height: 30px; background-color: yellow;'>";
  html += "</form>";

  html += "<script>";
  html += "function validateForm() {";
  html += "  var tempCValue = document.getElementById('tempC').value;";
  html += "  if (!tempCValue || isNaN(tempCValue)) {";
  html += "    alert('Please enter a valid numeric value for Temp correction.');";
  html += "    return false;";
  html += "  }";
  html += "}";
  html += "</script>";


  html += "<script>function goToRoot() { window.location.href = '/'; }</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleFourthPageSubmit() {
  String tempC = server.arg("tempC");

  // Assuming you have enough space in EEPROM for the tempC string
  writeStringToEEPROM(10, tempC);

  // Print the saved value to Serial
  Serial.println("Temp Correction: " + tempC);

  // Assuming you have some condition to determine success or failure
  bool success = true;  // Change this based on your actual condition

  String message;
  String color;

  if (success) {
    message = "Temperature correction saved successfully";
    color = "green";
  } else {
    message = "Failed to save data";
    color = "red";
  }

  String html = "<html><body>";
  html += "<h1 style='color:" + color + "'>" + message + "</h1>";
  html += "<button style='font-size: 30px; background-color: lightgreen;' onclick='goToFourthPage()'>Back</button>";
  html += "<script>function goToFourthPage() { window.location.href = '/Fourth'; }</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
/////////////////////////////////WIFI//////////////////////////////////////

void setup(void) {
  WiFi.softAP(ssid, password);
  delay(100);

  WiFi.softAPConfig(apIP, gatewayIP, subnetIP);

  server.begin();

  server.on("/", HTTP_GET, handleRoot);

  server.on("/second", HTTP_GET, handleSecondPage);

  server.on("/third", HTTP_GET, handleThirdPage);
  server.on("/submit", HTTP_POST, handleThirdPageSubmit);  // New route for form submission

  server.on("/Fourth", HTTP_GET, handleFourthPage);
  server.on("/submitFourth", HTTP_POST, handleFourthPageSubmit);

  server.on("/startRun", HTTP_POST, [](){
  State_Run = 1; // ตั้งค่า State_Run เมื่อได้รับ request
  server.send(200, "application/json", "{\"State_Run\": 1}");
  });

  server.on("/stopRun", HTTP_POST, [](){
    State_Run = 2; // ตั้งค่า State_Run เมื่อได้รับ request
    server.send(200, "application/json", "{\"State_Run\": 2}");
  });

  server.on("/resetRun", HTTP_POST, [](){
    State_Run = 4; // ตั้งค่า State_Run เมื่อได้รับ request
    server.send(200, "application/json", "{\"State_Run\": 4}");
  });

  //LCD
  u8g2.begin();

  // กลับจอ 180 องศา
  //u8g2.setDisplayRotation(U8G2_R2);

  //หน้าแรก
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);

    u8g2.setDrawColor(1);                // 1 for solid color
    u8g2.setFont(u8g2_font_crox4tb_tf);  // Use a bold font, you can choose a different one if needed

    // Set the position to display "Project NANO" on the screen
    int x = 2;   // X coordinate
    int y = 30;  // Y coordinate
    u8g2.drawStr(x, y, "Project NANO");
    int i = 45;  // X coordinate
    int z = 50;  // Y coordinate
    u8g2.drawStr(i, z, "v0.1");

  } while (u8g2.nextPage());
  delay(5000);  // รอ 1 วินาที

  //หน้าสอง
  Page2();
  delay(5000);  // รอ 1 วินาที

  EEPROM.begin(512);  // Begin with EEPROM size

  //wifi
  Serial.begin(115200);
  Serial.println("Access Point IP address: " + WiFi.softAPIP().toString());
  Serial.println("HTTP server started");
}

/////////////////////////////////LCD//////////////////////////////////////
void Page2(void) {
  u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);

    u8g2.setDrawColor(1);               // 1 for solid color
    u8g2.setFont(u8g2_font_luBS08_te);  // Use a bold font, you can choose a different one if needed
    // Set the position to display "Project NANO" on the screen
    int a = 28;  // X coordinate
    int b = 15;  // Y coordinate
    u8g2.drawStr(a, b, "Develop by");
    int c = 2;   // X coordinate
    int d = 30;  // Y coordinate
    u8g2.drawStr(c, d, "Chainarong Kulchim");

    u8g2.setFont(u8g2_font_baby_tf);  // Use a bold font, you can choose a different one if needed
    int f = 12;                       // X coordinate
    int g = 47;                       // Y coordinate
    u8g2.drawStr(f, g, "(Medical scientist : VRDSN)");

    u8g2.setFont(u8g2_font_profont10_mr);  // Use a bold font, you can choose a different one if needed
    int h = 6;                             // X coordinate
    int i = 57;                            // Y coordinate
    u8g2.drawStr(h, i, "Chainarong681@gmail.com");

  } while (u8g2.nextPage());
}

//Loading Bar
void draw_box(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t width, u8g2_uint_t height) {
  u8g2.drawBox(x, y, width, height);
}

void draw_loading_bar(u8g2_uint_t boxlong) {
  u8g2.firstPage();
  do {

    u8g2.setFont(u8g2_font_luBS08_te);  // Use a bold font, you can choose a different one if needed
    // Set the position to display "Project NANO" on the screen
    int x = 28;  // X coordinate
    int y = 20;  // Y coordinate
    u8g2.drawStr(x, y, "please wait");
    // Draw loading bar based on the value of boxlong
    draw_box(10, 27, boxlong, 10);
  } while (u8g2.nextPage());
  //delay(30);
}

/////////////////////////////////LCD//////////////////////////////////////
void loop(void) {
  //LCD
  //Loading bar
  if (boxlong < 108) {
    draw_loading_bar(boxlong);
    boxlong += 2;  // Increase boxlong or update it based on your logic
  } 
  else {
    //WIFI
    server.handleClient();

    //อ่านค่าตัวแปลไปเก็บเพื่อใช้เป็นค่า setting ในตอนกรณีที่เครื่องไม่อยู่ในสถานะการ Run ทำงาน
    if (State_Run != 1) {
      //อ่าน EEPROM เก็บที่ตัวแปล
      // อ่านค่าจาก EEPROM ที่ตำแหน่ง 0 และเก็บในตัวแปล methodRUN
      String methodRUN_EEPROM;
      EEPROM.get(0, methodRUN_EEPROM);
      methodRUN = methodRUN_EEPROM.toFloat();
      // อ่านค่าจาก EEPROM ที่ตำแหน่ง 5 (float มีขนาด 4 บิต) และเก็บในตัวแปล timeRUN
      String timeRUN_EEPROM;
      EEPROM.get(5, timeRUN_EEPROM);
      timeRUN = timeRUN_EEPROM.toFloat();
      // อ่านค่าจาก EEPROM ที่ตำแหน่ง 10 และเก็บในตัวแปล temCRUN
      String temCRUN_EEPROM;
      EEPROM.get(10, temCRUN_EEPROM);
      temCRUN = temCRUN_EEPROM.toFloat();
    }

     //Run program
    Run();
    Serial.println(State_Run);

    
  }
}

void Run() {
 u8g2.firstPage();
  do {
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.setDrawColor(1);                    // 1 for solid color
    u8g2.setFont(u8g2_font_squeezed_b6_tr);  // Use a bold font, you can choose a different one if needed
    // Set the position to display "Project NANO" on the screen
    int a = 2;  // X coordinate
    int b = 8;  // Y coordinate
    u8g2.drawStr(a, b, "Read:");
    int c = 30;  // X coordinate
    int d = 8;   // Y coordinate
    u8g2.setCursor(c, d);
    u8g2.print(READTEMP, 2);
    // Set the position to display "Project NANO" on the screen
    int e = 57;  // X coordinate
    int f = 8;   // Y coordinate
    u8g2.drawStr(e, f, "/ ");
    int g = 63;  // X coordinate
    int h = 8;   // Y coordinate
    u8g2.setCursor(g, h);
    u8g2.print(methodRUN, 2);

    //สร้าง status ตอนรัน
    if (State_Run == 0) {
      u8g2.setFont(u8g2_font_squeezed_b6_tr);  // Use a bold font, you can choose a different one if needed
      int i = 90;                              // X coordinate
      int j = 8;                               // Y coordinate
      u8g2.drawStr(i, j, "-->WAIT");
    }
    if (State_Run == 1) {
        //ตรวจสอบ Run complete
        if (TIME_COUNT <= timeRUN) {
          u8g2.setFont(u8g2_font_squeezed_b6_tr);  // Use a bold font, you can choose a different one if needed
          int i = 90;                              // X coordinate
          int j = 8;                               // Y coordinate
          u8g2.drawStr(i, j, "-->RUN");
        }
        //ตรวจสอบ Run complete
        if (TIME_COUNT >= timeRUN) {
          TIME_COUNT = timeRUN;
          u8g2.setFont(u8g2_font_micro_mr);  // Use a bold font, you can choose a different one if needed
          int i1 = 111;                              // X coordinate
          int j1 = 60;                               // Y coordinate
          u8g2.drawStr(i1, j1, "END");
        }
    }
    if (State_Run == 2) {
      u8g2.setFont(u8g2_font_squeezed_b6_tr);  // Use a bold font, you can choose a different one if needed
      int i = 90;                              // X coordinate
      int j = 8;                               // Y coordinate
      u8g2.drawStr(i, j, "-->STOP");
    } 

    //ส่วนแสดงกราฟ
    //ตัวเลข เลเบล
    u8g2.setFont(u8g2_font_4x6_mn);  // Use a bold font, you can choose a different one if needed
    // Set the position to display "Project NANO" on the screen
    int n1 = 3;  // X coordinate
    int b1 = 53;  // Y coordinate
    u8g2.drawStr(n1, b1, "30");
    int n2 = 3;  // X coordinate
    int b2 = 43;  // Y coordinate
    u8g2.drawStr(n2, b2, "40");
    int n3 = 3;  // X coordinate
    int b3 = 33;  // Y coordinate
    u8g2.drawStr(n3, b3, "50");
    int n4 = 3;  // X coordinate
    int b4 = 23;  // Y coordinate
    u8g2.drawStr(n4, b4, "60");

    //วาดแกน
    // วาดเส้นแนวตั้งที่ตำแหน่ง x=5, y=10, ความยาว 50 pixel
    u8g2.drawVLine(12, 10, 50);
    // วาดจุดแนวตั้งที่ตำแหน่ง x=20, y=10 และห่างกันทีละ 5 จุด
    for (int i = 0; i < 15; i++) {
      u8g2.drawPixel(11, 10 + i * 5);
    }
    // วาดเส้นแนวนอนที่ตำแหน่ง x=5, y=60, ความยาว 91 pixel
    u8g2.drawHLine(12, 60, 91);
    // วาดจุดแนวนอนที่ตำแหน่ง x=5, y=61 และห่างกันทีละ 5 จุด
    for (int i = 0; i < 19; i++) {
      u8g2.drawPixel(12 + i * 5, 61);
    }

    //กล่อง parameter
    u8g2.drawFrame(104, 10, 24, 52);
    u8g2.setFont(u8g2_font_micro_mr);  // Use a bold font, you can choose a different one if needed
    int l = 106;                       // X coordinate
    int m = 18;                        // Y coordinate
    u8g2.drawStr(l, m, "Time");
    int n = 112;  // X coordinate
    int o = 25;   // Y coordinate
    u8g2.setCursor(n, o);
    u8g2.print(TIME_COUNT);

    int p = 106;                       // X coordinate
    int q = 32;                        // Y coordinate
    u8g2.drawStr(p, q, "T-Set");
    int p1 = 106;  // X coordinate
    int q1 = 39;   // Y coordinate
    u8g2.setCursor(p1, q1);
    u8g2.print(timeRUN);

    int r = 106;                       // X coordinate
    int s = 46;                        // Y coordinate
    u8g2.drawStr(r, s, "PID");
    int p1w = 106;  // X coordinate
    int q1w = 53;   // Y coordinate
    u8g2.setCursor(p1w, q1w);
    u8g2.print(timeRUN);


  //ตรวจสอบว่าอยู่ในสถานะ Run หรือไม่
      if (State_Run == 0) { //WAIT
        //อ่านค่าและจับเวลา
        timer_wait();     
      } else if(State_Run == 1) { //RUN
        timer_RUN();
        //อ่านกราฟ
        Graph();
        // //ตรวจสอบ Run complete
        // if (TIME_COUNT >= timeRUN) {
        //   TIME_COUNT = timeRUN;
        // }
      }
      else if(State_Run == 2) { //STOP
        //โค้ดจะไปรบกวนการสร้างให้หยุดลง เมื่อกด Run ระบบจะทำการบันทึกต่อไปได้เลย
       
      }
      else if(State_Run == 4) { //Reset
        currentIndex =0; //รีเซ็ทค่า currentIndex เริ่มต้นใหม่
        clearGraph();
        State_Run = 0;
      }

} while (u8g2.nextPage());
 
}

void timer_wait(){
  if (millis() - last_time > period) {
    TIME_COUNT = 0;
    //ทดสอบ random ค่า
    READTEMP = random(500, 660) / 10.0;
    last_time = millis();  //เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
  } 
}

void timer_RUN() {
  if (millis() - last_time > period) {
    //ทดสอบ random ค่า
    READTEMP = random(500, 660) / 10.0;

    TIME_COUNT++;

    //ส่วนสร้างกราฟ
    if (currentIndex < timeRUN){ //เก็บค่าแค่ 90 ค่าเท่านั้น
      currentIndex++;
      
      //Serial.println(currentIndex);
    //ทดสอบ random ค่า
    temperature[currentIndex] = READTEMP;  // Replace with actual sensor reading
    } 
    last_time = millis();  //เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
  }
}

void Graph() { //กราฟ scatter plot
  for (int igraph = 0; igraph < timeRUN; igraph++) {
    int yPos1 = map(temperature[igraph], 25, 70, 55, 10);  
    int yPos2 = map(temperature[igraph + 1], 25, 70, 55, 10);

    // ใช้ u8g2.drawHLine() แทน u8g2.drawLine() หรือลองใช้คำสั่งวาดเส้น曲จากศูนย์กลาง
    u8g2.drawHLine(12 + igraph, yPos1, 1); 
  }
}

void clearGraph() { //เคลียร์หน้าจอ
  for (int i = 0; i < timeRUN; i++) {
    temperature[i] = 0.0;
  }
}

////////////////////////////////////กราฟโค้ด///////////////////////////////////////////////////////////////
// void Graph(){ //กราฟเส้น
//   //ส่วนสร้างกราฟ
//     for (igraph = 0; igraph< 89; igraph++) {
//       int yPos1 = map(temperature[igraph], 25, 70, 55, 10);  // Map temperature to fit Y-axis range
//       int yPos2 = map(temperature[igraph + 1], 25, 70, 55, 10);
//       u8g2.drawLine(5 + igraph, yPos1, 5 + igraph, yPos2);   
//     }

// }

// void Graph() { //กราฟ scatter plot
//   for (int igraph = 0; igraph < 89; igraph++) {
//     int yPos1 = map(temperature[igraph], 25, 70, 55, 10);  
//     int yPos2 = map(temperature[igraph + 1], 25, 70, 55, 10);

//     // ใช้ u8g2.drawHLine() แทน u8g2.drawLine() หรือลองใช้คำสั่งวาดเส้น曲จากศูนย์กลาง
//     u8g2.drawHLine(5 + igraph, yPos1, 1); 
//   }
// }

// void Graph() { //กราฟเส้น
//   for (int igraph = 0; igraph < 89; igraph++) {
//     int x1 = 4 + igraph;
//     int y1 = map(temperature[igraph], 25, 70, 55, 10);

//     int x2 = 5 + igraph;
//     int y2 = map(temperature[igraph + 1], 25, 70, 55, 10);

//     u8g2.drawLine(x1, y1, x2, y1);  // วาดเส้นแนวนอนที่ y คงที่
//     u8g2.drawLine(x2, y1, x2, y2);  // วาดเส้นแนวตั้งไปยัง y ถัดไป
//   }
// }

// void Graph() { //กราฟแท่ง
//   for (int igraph = 0; igraph < 89; igraph++) {
//     int yPos = map(temperature[igraph], 25, 70, 55, 10); // ปรับค่าใน map()
//     // ใช้ u8g2.drawVLine() เพื่อวาดแท่ง
//     u8g2.drawVLine(5 + igraph, yPos, 30 + yPos);
//   }
// }
////////////////////////////////////โค้ด Timer ต้นแบบ///////////////////////////////////////////////////////////////
// void timer() {
//   if (millis() - last_time > period) {

//     TIME_COUNT++;

//     //ส่วนสร้างกราฟ
//     if (currentIndex <89){ //เก็บค่าแค่ 90 ค่าเท่านั้น
//       currentIndex++;
//       //Serial.println(currentIndex);
      
//     //ทดสอบ random ค่า
//     temperature[currentIndex] = READTEMP;  // Replace with actual sensor reading

//     } 
//     else if (currentIndex >=89){ //รีเซ็ทค่า currentIndex
//       currentIndex =0; //รีเซ็ทค่า currentIndex เริ่มต้นใหม่
//       clearGraph();
//     }
//     last_time = millis();  //เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period
//     if (TIME_COUNT >= timeRUN) {
//       TIME_COUNT = 0;
//     }
//   }
// }
