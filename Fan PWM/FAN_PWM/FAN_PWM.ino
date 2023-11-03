int fanPin = 9;  // กำหนดขาที่เชื่อมกับสายสัญญาณ PWM ของพัดลม

void setup() {
  pinMode(fanPin, OUTPUT);  // ตั้งค่าขาเป็นขาส่งออก
}

void loop() {
  // ควบคุมความเร็วของพัดลมด้วย PWM
  int fanSpeed = 225;  // ความเร็วพัดลมระหว่าง 0 ถึง 255
  analogWrite(fanPin, fanSpeed);
  delay(1000);  // หน่วงเวลา 1 วินาที
}
