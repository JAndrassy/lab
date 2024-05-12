
/**
 * This sketch emulates a TM1637 IC
 * It works as slave for a host MCU
 * with TM1637Display library by Avishay Orpaz
 *
 * It is useful to outsource handling of complicated displays
 * to a separate MCU and use a standard library in the host MCU.
 */

#define CLK A4
#define DIO A5

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

const int8_t gnd1 = 0;
const int8_t gnd2 = 1;

const int8_t to6 = 4;
const int8_t to7 = 5;
const int8_t to8 = 6;
const int8_t to9 = 7;
const int8_t to10 = 8;
const int8_t to12 = 9;
const int8_t to13 = 10;
const int8_t to15 = 11;
const int8_t to16 = 12;
const int8_t to17 = 3;
const int8_t to18 = 14;
const int8_t to19 = 15;
const int8_t to20 = 16;
const int8_t to21 = 17;
const int8_t toColon = 2;

//      0
//     ---
//  5 |   | 1
//     -6-
//  4 |   | 2
//     ---
//      3

int8_t pinMap[4][8] = { //
    {to7, -to6, -to9, to8, -to8, to6, -to7, 0}, //
    {-to13, -to10, -to12, to12, to9, to13, to10, toColon}, //
    {to15, to16, to17, -to17, -to18, -to15, -to16, 0}, //
    {to21, -to19, -to20, to20, to18, to21, to19, 0} //
};

byte data[4];
byte brightness = 0;
uint32_t lastReceiveMillis;

void setup() {
  //Serial.begin(115200);

  pinMode(CLK, INPUT_PULLUP);
  pinMode(DIO, INPUT_PULLUP);

  pinMode(gnd1, OUTPUT);
  pinMode(gnd2, OUTPUT);
  digitalWrite(gnd1, LOW);
  digitalWrite(gnd2, LOW);

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 7; j++) {
      pinMode(abs(pinMap[i][j]), OUTPUT);
      digitalWrite(abs(pinMap[i][j]), LOW);
    }
  }
}

void loop() {
  refresh();
  if (millis() - lastReceiveMillis > 70 && digitalRead(DIO) == LOW && digitalRead(CLK) == HIGH) {
    lastReceiveMillis = millis();
    byte b;
    if (!readByte(b, 0))
      return;
    if (b != TM1637_I2C_COMM1) {
      //Serial.print("abort at COMM1. received 0x");
      //Serial.println(b, HEX);
      return;
    }
    if (!stop(90))
      return;

    if (!(start(100) && readByte(b, 100)))
      return;
    if ((b & ~0x03) != TM1637_I2C_COMM2) {
      //Serial.print("abort at COMM2. received 0x");
      //Serial.println(b, HEX);
      return;
    }
    byte temp[4];
    if (!(readByte(temp[0], 200) && readByte(temp[1], 300) && readByte(temp[2], 400) && readByte(temp[3], 500)))
      return;
    if (!stop(590))
      return;

    if (!(start(600) && readByte(b, 600)))
      return;
    if ((b & ~0x0F) != TM1637_I2C_COMM3) {
      //Serial.print("abort at COMM3. received 0x");
      //Serial.println(b, HEX);
      return;
    }
    if (!stop(690))
      return;

    brightness = b & 0x0F;
    for (int i = 0; i < 4; i++) {
      data[i] = temp[i];
    }
  }
}

bool start(int debugId) {
  return waitChange(DIO, LOW, debugId);
}

bool stop(int debugId) {
  return (waitChange(CLK, HIGH, debugId) && waitChange(DIO, HIGH, debugId + 10));
}

bool readByte(byte &b, int debugId) {
  b = 0;
  for (int i = 0; i < 8; i++) {
    if (!waitChange(CLK, LOW, debugId + 10))
      return false;
    uint32_t m = micros();
    refresh();
    int32_t d = 120L - (micros() - m);
    if (d > 0) {
      delayMicroseconds(d);
    }
    if (digitalRead(DIO) == HIGH) {
      b |= (1 << i);
    }
    if (!waitChange(CLK, HIGH, debugId + 20))
      return false;
  }
  if (!waitChange(CLK, LOW, debugId + 30))
    return false;
  if (!waitChange(CLK, HIGH, debugId + 40))
    return false;
  if (!waitChange(CLK, LOW, debugId + 50))
    return false;
//    pinMode(DIO, OUTPUT);
  //  digitalWrite(DIO, LOW);
  //    pinMode(DIO, INPUT_PULLUP);
  return b;
}

bool waitChange(int pin, int changeTo, int debugId) {
  uint32_t startMicros = micros();
  while (micros() - startMicros < 500) {
    if (digitalRead(pin) == changeTo)
      return true;
  }
  //Serial.print("abort at ");
  //Serial.println(debugId);
  return false;
}

void refresh() {
  static uint32_t lastUpdateMillis;
  static uint8_t side = 0;
  static uint8_t phase = 0;

  if (millis() - lastUpdateMillis > 10) {
    lastUpdateMillis = millis();
    draw((side * 2), phase, false);
    draw((side * 2) + 1, phase, false);
    if (phase == 1) {
      side = !side;
    }
    phase = !phase;
    digitalWrite(gnd1, phase);
    digitalWrite(gnd2, !phase);
    if (brightness > 0) {
      draw((side * 2), phase, true);
      draw((side * 2) + 1, phase, true);
    }
  }
}

void draw(uint8_t digit, uint8_t phase, bool on) {
  for (int segment = 0; segment < 8; segment++) {
    int8_t p = pinMap[digit][segment];
    if (p == 0 || phase == (p < 0))
      continue;
    if (data[digit] & (1 << segment)) {
      digitalWrite(abs(p), on);
    }
  }
}