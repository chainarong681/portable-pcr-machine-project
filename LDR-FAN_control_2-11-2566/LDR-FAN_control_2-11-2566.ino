int fanPin = 9;  // กำหนดขาที่เชื่อมกับสายสัญญาณ PWM ของพัดลม
int analogPin = 5; //ประกาศตัวแปร ให้ analogPin แทนขา analog ขาที่5
int val = 0;

void setup() {
  pinMode(fanPin, OUTPUT);  // ตั้งค่าขาเป็นขาส่งออก
  Serial.begin(9600);
}

void loop() {
  //LDR
  val = analogRead(analogPin);  //อ่านค่าสัญญาณ analog ขา5 ที่ต่อกับ LDR
  Serial.print("val = "); // พิมพ์ข้อมความส่งเข้าคอมพิวเตอร์ "val = "
  Serial.println(val); // พิมพ์ค่าของตัวแปร val
  
  if (val < 150) { // ค่า 100 สามารถกำหนดปรับได้ตามค่าแสงในห้องต่างๆ
    // ควบคุมความเร็วของพัดลมด้วย PWM
    int fanSpeed = 225;  // ความเร็วพัดลมระหว่าง 0 ถึง 255 = 100%
    analogWrite(fanPin, fanSpeed);
  }
  else if (val >= 150 && val < 380){
    // ควบคุมความเร็วของพัดลมด้วย PWM
    int fanSpeed = 128;  // ความเร็วพัดลมระหว่าง 0 ถึง 255 = 50%
    analogWrite(fanPin, fanSpeed); 
  }
  else if (val >= 380 && val < 900){
    // ควบคุมความเร็วของพัดลมด้วย PWM
    int fanSpeed = 0;  // ความเร็วพัดลมระหว่าง 0 ถึง 255 =หมุนตามปรกติ Fan CPU ไม่หยุด
    analogWrite(fanPin, fanSpeed); 
  }
  delay(1000);  // หน่วงเวลา 1 วินาที
}
