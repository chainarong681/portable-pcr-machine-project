#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

MDNSResponder mdns;
ESP8266WebServer server(80);
//IP connection
const char* ssid = "TempAlarm01_V2.0"; //ssid ที่แสดง
const char* passphrase = "123456789"; //ใส่รหัสเพื่อเชื่อมต่อกับ ESP8266 ที่ IP 192.168.4.1
String st;
String content;


//Internal LED blink when DS18B20 read TEMP
#define LED1 D0 //Define blinking LED pin
//LED open when WIFI connected.
#define LED2 D2 //Define blinking LED pin
//LED open when WIFI not connected.
#define LED3 D3 //Define blinking LED pin
int ledState1 = LOW; //Read TEMP
int ledState2 = LOW; //WIFI connection
int ledState3 = LOW; //WIFI not connection
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;

// Config connect WiFi
String esidString;
String epassString;
String ChanelIDString;
String apiKeyString;
String lineTokenString;
String blynkTokenString;
String correctionTempString;
String lineWarningString;
String lineReportString;

//Thingspeak
#include "ThingSpeak.h" //Thingspeak librarry
WiFiClient client;

// Line config
#include <TridentTD_LineNotify.h>
String message1 = "อุณหภูมิอยู่ในเกณฑ์กำหนดมีค่าเท่ากับ ";
String message2 = "คำเตือน!...อุณหภูมิไม่อยู่ในเกณฑ์กำหนดมีค่าสูงกว่าเกณฑ์เท่ากับ ";

 //Blynk
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>


//Temp DS18B20 probe
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D1 // Data wire is plugged into pin D2
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices // (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
float room_temp;
String tempString;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(250); //ใช้ EEPROM ขนาดเท่านี้
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //internal LED pin Blank when DS18B20 read TEMP
  pinMode(LED1, OUTPUT); 

  //LED open when WIFI connected.
  pinMode(LED2, OUTPUT);

   //LED open when WIFI not connected.
  pinMode(LED3, OUTPUT);

  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  
  //1. wifi SSID and PASS to EEPROM
  String esid;
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 96; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  Serial.print("PASS: ");
  Serial.println(epass);

  //2. Thingspeak to EEPROM
  String ChanelID;
  for (int i = 96; i < 106; ++i)
    {
      ChanelID += char(EEPROM.read(i));
    }
  Serial.print("Channel ID: ");
  Serial.println(ChanelID);
  Serial.println("Reading EEPROM pass");
  String apiKey = "";
  for (int i = 106; i < 126; ++i)
    {
      apiKey += char(EEPROM.read(i));
    }
  Serial.print("API Key: ");
  Serial.println(apiKey);

//3. Line to EEPROM
  String lineToken;
  for (int i = 126; i < 176; ++i)
    {
      lineToken += char(EEPROM.read(i));
    }
  Serial.print("Line Token: ");
  Serial.println(lineToken);
  Serial.println("Reading EEPROM pass");

//4. Blynk to EEPROM
  String blynkToken;
  for (int i = 176; i < 211; ++i)
    {
      blynkToken += char(EEPROM.read(i));
    }
  Serial.print("BLYNK Token: ");
  Serial.println(blynkToken);
  Serial.println("Reading EEPROM pass");

//5. Corection temp to EEPROM
  String correctionTemp;
  for (int i = 211; i < 221; ++i)
    {
      correctionTemp += char(EEPROM.read(i));
    }
  Serial.print("Corection TEMP: ");
  Serial.println(correctionTemp);
  Serial.println("Reading EEPROM pass");

  //6. Line time warning to EEPROM
  String lineWarning;
  for (int i = 221; i < 231; ++i)
    {
      lineWarning += char(EEPROM.read(i));
    }
  Serial.print("Line time warning: ");
  Serial.println(lineWarning);
  Serial.println("Reading EEPROM pass");

  //7. Line time report to EEPROM
  String lineReport;
  for (int i = 231; i < 241; ++i)
    {
      lineReport += char(EEPROM.read(i));
    }
  Serial.print("Line time Report: ");
  Serial.println(lineReport);
  Serial.println("Reading EEPROM pass");

  //if SSID and PASS are connected(is not empty).
  if ( esid.length() > 1 ) {
      // test esid (convert all to String)
      WiFi.begin(esid.c_str(), epass.c_str()); //Wifi setting

      //set parameter for serial print.
      esidString = esid.c_str();
      epassString = epass.c_str();
      ChanelIDString = ChanelID.c_str();
      apiKeyString = apiKey.c_str();
      lineTokenString = lineToken.c_str();
      blynkTokenString = blynkToken.c_str();
      correctionTempString = correctionTemp.c_str();
      lineWarningString = lineWarning.c_str();
      lineReportString = lineReport.c_str();

      //ThingSpeak begin
      ThingSpeak.begin(client);

      //Line setting
      LINE.setToken(lineTokenString.c_str());
      Serial.println(LINE.getVersion());

//      //Blynk
//      Blynk.begin((char*)blynkTokenString.c_str(),(char*)esidString.c_str(),(char*)epassString.c_str());
//      //เปลี่ยน string จากตัวแปล เช่น blynkTokenString เป็น String โดยใช้ .c_str() และเปลี่ยนใส่เป็น char ใน array โดยใช้ (char*) ข้างหน้า
      Blynk.config((char*)blynkTokenString.c_str());  //ใช้แทน Blynk.begin ใช้แค่ Token
     
      if (testWifi()) { 
          launchWeb(0);
          return;
      }
  }
 
  //Temp DS18B20
  sensors.begin();  // Start up the library
  
  setupAP();
}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; }
    delay(500);
    Serial.print(WiFi.status());    
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
} 

void launchWeb(int webtype) {
  //launch web  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  if (!mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started"); 
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started"); 
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     }
  }
  Serial.println(""); 
  st = "<ol>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</li>";
    }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid,passphrase); //เชื่อมต่อกับ ESP8266 เพื่อตั้งค่าที่ 192.168.4.1 โดยการเรียกรหัสเชื่อมต่อ
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype)//เขียนหน้าเวป............................................................
{
  // Check for any mDNS queries and send responses
  mdns.update();
  
  if ( webtype == 1 ) {
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>";
        content = "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> <div style='background-color:green;color:white;padding:4px;'> <h1 style='text-align:center;'>TempAlarm V2.0</h1> <h4 style='text-align:center;'>(มีปัญหาการใช้งานติดต่อ นายชัยณรงค์ กุลฉิม tel. 080-5040716)</h4> </div><body style='background-color:white;'>";  //แถบหัวและพื้นหลัง
        content += "<p>IP addrese เชื่อมต่อระบบ : " + ipStr;
        content += "<p>"; //บันทัดเปล่า
        content += st;
        //<br /> คือ เว้นบันทัด
        //&nbsp คือเว้นวรรค
        content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> </p><form method='get' action='setting'> <form><fieldset style='background-color: #dfdfdf; border: 2px solid green; width: 750px'><legend>ตั้งค่าเชื่อมต่อ WiFi:</legend> <label>SSID: </label><input name='ssid' length=32>&nbsp<label>PASS: </label><input name='pass' length=64>&nbsp<input type='submit' value='บันทึกข้อมูล'> </fieldset> </form><p><strong><font color ='red'>Note:</strong> ตัวเลขใน ( )* หลัง SSID มีค่าน้อยที่สุดหมายถึงค่าความแรงสัญญาณ<สูงที่สุด>.</font></p></form>";
        content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> </p><form method='get' action='setting'> <form><fieldset style='background-color: #dfdfdf; border: 2px solid green; width: 750px'><legend>ตั้งค่าเชื่อมต่อ Thingspeak:</legend> <label>Channel ID: </label><input name='ch_id' length=32>&nbsp<label>API Key: </label><input name='api_key' length=64>&nbsp<input type='submit' value='บันทึกข้อมูล'> </fieldset> </form>";
        content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> </p><form method='get' action='setting'> <form><fieldset style='background-color: #dfdfdf; border: 2px solid green; width: 750px'><legend>ตั้งค่าเชื่อมต่อ Line message:</legend> <label>Line Token: </label><input name='lineTokenK' length=32>&nbsp<input type='submit' value='บันทึกข้อมูล'> </fieldset> </form>";
        content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> </p><form method='get' action='setting'> <form><fieldset style='background-color: #dfdfdf; border: 2px solid green; width: 750px'><legend>ตั้งค่าเชื่อมต่อ Blynk:</legend> <label>Blynk Token: </label><input name='BlynkTokenK' length=32>&nbsp<input type='submit' value='บันทึกข้อมูล'> </fieldset> </form>";
        content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> </p><form method='get' action='setting'> <form><fieldset style='background-color: #dfdfdf; border: 2px solid green; width: 750px'><legend>ตั้งค่าระบบวัด/ระบบรายงานผล:</legend> <label>1. ค่าแก้อุณหภูมิ: </label><input name='correctTempV' length=32>&nbsp<input type='submit' value='บันทึกข้อมูล'></form><p><strong><font color ='red'>Note:</strong>ค่าแก้ = -(ค่าที่อ่านได้จากเครื่องมือ - ค่าจริง)โดยการตั้งค่ามีหน่วยเป็นองศาเซลเซียส. </font></p><form method='get' action='setting'><label>2. อุณหภูมิแจ้งเตือนจุดวิกฤต: </label><input name='criticalTempV' length=32>&nbsp<input type='submit' value='บันทึกข้อมูล'></form><p><strong><font color ='red'>Note:</strong>แจ้งเตือนผ่านไลน์เมื่ออุณหภูมิถึงจุดวิกฤตที่กำหนดทุกๆ 15 นาที โดยการตั้งค่ามีหน่วยเป็นองศาเซลเซียส. </font></p><form method='get' action='setting'><label>3. เวลารายงานสถานะอุณหภูมิในสภาวะปรกติ: </label><input name='normalTempV' length=32>&nbsp<input type='submit' value='บันทึกข้อมูล'></form><p><strong><font color ='red'>Note:</strong>แจ้งข้อความผ่านไลน์ตามช่วงเวลาที่กำหนดในสภาวะปรกติ โดยการตั้งค่ามีหน่วยเป็นนาที. </font></p></form>";
        content += "</body></html>";
        server.send(200, "text/html", content);
    });
    
    server.on("/setting", []() {
    //EEPROM
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    String ThingChanelID = server.arg("ch_id");
    String ThingapiKey = server.arg("api_key");
    String lineTokenKey = server.arg("lineTokenK");
    String blynkTokenKey = server.arg("BlynkTokenK");
    String correctionTempValue = server.arg("correctTempV");
    String lineWarningValue= server.arg("criticalTempV");
    String lineReportValue = server.arg("normalTempV");

    if (qsid.length() == 0 || qpass.length() == 0 || ThingChanelID.length() == 0 || ThingapiKey.length() == 0 || lineTokenKey.length() == 0 || blynkTokenKey.length() == 0 || correctionTempValue.length() == 0 || lineWarningValue.length() == 0 || lineReportValue.length() == 0) {
         content = "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> คุณใส่ข้อมูลผิดพลาด !โปรดตรวจสอบข้อมูลอีกครั้ง";
         Serial.println("Sending 404");
         }
        
    if (qsid.length() > 0 && qpass.length() > 0) {
       //1. wifi                    
          
          Serial.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
    
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]); 
            }
          Serial.println("writing eeprom pass:"); 
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]); 
            }
                                
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าการเชื่อมต่อ WI-FI อีกครั้ง</p></html>";
    }
    
    if (ThingChanelID.length() > 0 && ThingapiKey.length() > 0) {
     //2.Thingspeak
              
          Serial.println("clearing eeprom");
          for (int i = 96; i < 126; ++i) { EEPROM.write(i, 0); }
          Serial.println(ThingChanelID);
          Serial.println("");
          Serial.println(ThingapiKey);
          Serial.println("");
            
          Serial.println("writing eeprom ThingChanelID:");
          for (int i = 0; i < ThingChanelID.length(); ++i)
            {
              EEPROM.write(96+i, ThingChanelID[i]);
              Serial.print("Wrote: ");
              Serial.println(ThingChanelID[i]); 
            }
          Serial.println("writing eeprom ThingapiKey:"); 
          for (int i = 0; i < ThingapiKey.length(); ++i)
            {
              EEPROM.write(106+i, ThingapiKey[i]);
              Serial.print("Wrote: ");
              Serial.println(ThingapiKey[i]); 
            }    
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าการเชื่อมต่อฐานข้อมูลออนไลน์ Thingspeak อีกครั้ง</p></html>";
    }
    
    if (lineTokenKey.length() > 0) {
       //3. Line measage
               
          Serial.println("clearing eeprom");
          for (int i = 126; i < 176; ++i) { EEPROM.write(i, 0); }
          Serial.println(lineTokenKey);
          Serial.println("");
                      
          Serial.println("writing eeprom lineTokenKey:");
          for (int i = 0; i < lineTokenKey.length(); ++i)
            {
              EEPROM.write(126+i, lineTokenKey[i]);
              Serial.print("Wrote: ");
              Serial.println(lineTokenKey[i]); 
            }
          Serial.println("writing eeprom lineTokenKey:"); 
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าการแจ้งเตือนผ่าน ไลน์ อีกครั้ง</p></html>";
    }
    
    if (blynkTokenKey.length() > 0) {
      //4. BLYNK
               
          Serial.println("clearing eeprom");
          for (int i = 176; i < 211; ++i) { EEPROM.write(i, 0); }
          Serial.println(blynkTokenKey);
          Serial.println("");
                      
          Serial.println("writing eeprom blynkTokenKey:");
          for (int i = 0; i < blynkTokenKey.length(); ++i)
            {
              EEPROM.write(176+i, blynkTokenKey[i]);
              Serial.print("Wrote: ");
              Serial.println(blynkTokenKey[i]); 
            }
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าการเชื่อมต่อ BLYNK</p></html>";
    }
    
    if (correctionTempValue.length() > 0) {
       //5. Corection TEMP
                
          Serial.println("clearing eeprom");
          for (int i = 211; i < 221; ++i) { EEPROM.write(i, 0); }
          Serial.println(correctionTempValue);
          Serial.println("");
                     
          Serial.println("writing eeprom correctionTempValue:");
          for (int i = 0; i < correctionTempValue.length(); ++i)
            {
              EEPROM.write(211+i, correctionTempValue[i]);
              Serial.print("Wrote: ");
              Serial.println(correctionTempValue[i]); 
            }
         
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าแก้ในการอ่านอุณหภูมิ</p></html>";
    }
    
    if (lineWarningValue.length() > 0) {
      //6. Line warning
                
          Serial.println("clearing eeprom");
          for (int i = 221; i < 231; ++i) { EEPROM.write(i, 0); }
          Serial.println(lineWarningValue);
          Serial.println("");
                      
          Serial.println("writing eeprom lineWarningValue:");
          for (int i = 0; i < lineWarningValue.length(); ++i)
            {
              EEPROM.write(221+i, lineWarningValue[i]);
              Serial.print("Wrote: ");
              Serial.println(lineWarningValue[i]); 
            }
          
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าการแจ้งเตือนอุณหภูมิวิกฤตผ่านแอปพิเคชันไลน์</p></html>";
    }
    
    if (lineReportValue.length() > 0) {
      //7. Line time Report
                
          Serial.println("clearing eeprom");
          for (int i = 231; i < 241; ++i) { EEPROM.write(i, 0); }
          Serial.println(lineReportValue);
          Serial.println("");
                      
          Serial.println("writing eeprom lineReportValue:");
          for (int i = 0; i < lineReportValue.length(); ++i)
            {
              EEPROM.write(231+i, lineReportValue[i]);
              Serial.print("Wrote: ");
              Serial.println(lineReportValue[i]); 
            }
          
          EEPROM.commit();
          content = "<!DOCTYPE HTML>\r\n<html>";
          content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /><p>บันทึกข้อมูลเรียบร้อยแล้ว โปรดกดปุ่ม reset เพื่อการตั้งค่าการรายงานผลอุณหภูมิในสถานะปรกตผ่านแอปพิเคชันไลน์ิ</p></html>";
    }
      
    server.send(200, "text/html", content); 
         
    });
  } else { //Reset ssid and password in EEPROM.
    server.on("/", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content = "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> <div style='background-color:red;color:black;padding:32px;'> <h1 style='text-align:left;'>ระบบได้เชื่อมต่อไวไฟสมบูรณ์แล้ว</h1> <h2 style='text-align:left;'>ถ้าต้องการแก้ไขรหัสเชื่อมต่อให้กดปุ่ม reset ด้านล่าง (ใส่รหัส 1234) และทำการ boot เข้าระบบใหม่อีกครั้ง</h2> </div><body style='background-color:white;'>";  //แถบหัวและพื้นหลัง
      content += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' /> </p><form method='get' action='setting'> <form></label><input name='pass1234' length=4>&nbsp<input type='submit' value='ปุ่ม RESET'></form>";
      content += "</body></html>";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
    String qreset = server.arg("pass1234");
      if (qreset == "1234") {  //String check...
          Serial.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          Serial.println(qreset);
          Serial.println("writing value 0 to eeprom pass (position 0 to 96:");                        
          EEPROM.commit();
      }
      server.send(200, "text/html", content); 
    }); 
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  //Read Temp
  LEDwifiConnection();
  
  //Line sending
  //1. Normal sendin every time
  static unsigned long TimeNormal = millis();
  if ((millis() - TimeNormal) > (lineReportString.toFloat() * 61234) && room_temp <= lineWarningString.toFloat()) { //delay time every time.
    TimeNormal = millis();
    LINE.notify(message1+ tempString);
    // เลือก Line Sticker ได้จาก https://devdocs.line.me/files/sticker_list.pdf
    LINE.notifySticker(2,179);        // ส่ง Line Sticker ด้วย PackageID 2 , StickerID 40
  }
  //2. Line message warning
  static unsigned long Time_warning = millis();
  if (room_temp >= lineWarningString.toFloat() && (millis() - Time_warning) > 902468) {//Every 15 minutes.
    Time_warning = millis();
    LINE.notify(message2 + tempString);
    // เลือก Line Sticker ได้จาก https://devdocs.line.me/files/sticker_list.pdf
    LINE.notifySticker(4,274);       // ส่ง Line Sticker ด้วย PackageID 4 , StickerID 274
  }

  //3. ThingSpeak send DATA
  static unsigned long Time_thingspeak = millis();
  if ((millis() - Time_thingspeak) > 305612) { //Send every 5 minutes.
     Time_thingspeak = millis();
     ThingSpeak.writeField(ChanelIDString.toInt(), 1, room_temp,apiKeyString.c_str());
     
     Blynk.virtualWrite(V1, room_temp); //BLYNK send data
     
  }

  //4. Temp DS18B20
  static unsigned long Time_ds18b20 = millis();
  if ((millis() - Time_ds18b20) > 5000) { //Temp warning limite every 1/2 hr.
     Time_ds18b20 = millis();
     sensors.requestTemperatures(); // Send the command to get temperatures
      //protect from nois and value error
        float room_temp_raw;
        room_temp_raw = sensors.getTempCByIndex(0);
          if (room_temp_raw != -127){
            room_temp = room_temp_raw + correctionTempString.toFloat(); //TEMP value
            Serial.println(room_temp);
            tempString = String(room_temp);  //Temp string
            LEDreadTemp();
         }
  }
  //Blynk
   Blynk.run();
}

void LEDreadTemp(void){
  //Time count for Loop: //Best code for replacemnt of delay frunction and Internal LED blink
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis1 >= 1000) {
    previousMillis1 = currentMillis;
    if (ledState1 == LOW) {
      ledState1 = HIGH;  // Note that this switches the LED *off* 
    } else {
      ledState1 = LOW;  // Note that this switches the LED *on*
    }
    digitalWrite(LED1,ledState1);
  }
}

void LEDwifiConnection(void){
  //Time count for Loop: WIFI connected
  if (WiFi.status() == WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis2 >= 1000) {
      previousMillis2 = currentMillis;      
      if (ledState2 == LOW) {
        ledState2 = HIGH;  // Note that this switches the LED *off*
        digitalWrite(LED3,LOW);
      } else {
        ledState2 = LOW;  // Note that this switches the LED *on*
      }
    digitalWrite(LED2,ledState2);
    }
  }else{
    //WIFI not connected.
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis3 >= 1000) {
     previousMillis3 = currentMillis;
      if (ledState3 == LOW) {
        ledState3 = HIGH;  // Note that this switches the LED *off*
        digitalWrite(LED2,LOW);
      } else {
        ledState3 = LOW;  // Note that this switches the LED *on*
      }
      digitalWrite(LED3,ledState3);
    } 
  }
}
