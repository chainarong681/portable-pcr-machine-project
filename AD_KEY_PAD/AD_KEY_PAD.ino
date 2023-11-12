const byte NONE = 0;
const byte LEFT = 1;
const byte UP = 2;
const byte RIGHT = 3;
const byte DOWN = 4;
const byte SELECT = 5;

const byte keypadPin = A15;
byte key = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println(F("Start\n"));
  pinMode(keypadPin, INPUT_PULLUP);
}

void loop() {
  key = getKey();
  if (key == LEFT) {
    Serial.println("LEFT");
  }
  if (key == RIGHT) {
    Serial.println("RIGHT");
  }
  if (key == UP) {
    Serial.println("UP");
  }
  if (key == DOWN) {
    Serial.println("DOWN");
  }
  if (key == SELECT) {
    Serial.println("SELECT");
  }
  //delay(100);
}

byte getKey() {
  int val = 0;
  byte button = NONE; // ตั้งค่าเริ่มต้นเป็น NONE
  val = analogRead(keypadPin);
  if (val <= 20) {
    button = LEFT;
  } else if ((val >= 100) && (val <= 200)) {
    button = UP;
  } else if ((val >= 300) && (val <= 400)) {
    button = DOWN;
  } else if ((val >= 500) && (val <= 600)) {
    button = RIGHT;
  } else if ((val >= 700) && (val <= 800)) {
    button = SELECT;
  }
  return button;
}
