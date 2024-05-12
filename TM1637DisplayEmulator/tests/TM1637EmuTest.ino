#include <StreamLib.h>

#define CLK A4
#define DIO A5

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

char buff[150];
CStringBuilder sb(buff, sizeof(buff));

uint32_t lastReceiveMillis;

byte data[4];
byte brightness;

void setup() {
  Serial.begin(115200);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DIO, INPUT_PULLUP);
}

void loop() {

  if (millis() - lastReceiveMillis > 70 && digitalRead(DIO) == LOW && digitalRead(CLK) == HIGH) {
    lastReceiveMillis = millis();
    sb.reset();
    byte b;
    if (!readByte(b, 0))
      return;
    if (b != TM1637_I2C_COMM1) {
      Serial.print("abort at COMM1. received 0x");
      Serial.println(b, HEX);
      return;
    }
    if (!stop(90))
      return;

    if (!(start(100) && readByte(b, 100)))
      return;
    if ((b & ~0x03) != TM1637_I2C_COMM2) {
      Serial.print("abort at COMM2. received 0x");
      Serial.println(b, HEX);
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
      Serial.print("abort at COMM3. received 0x");
      Serial.println(b, HEX);
      return;
    }
    if (!stop(690))
      return;

    brightness = b & 0x0F;
    for (int i = 0; i < 4; i++) {
      data[i] = temp[i];
    }

    Serial.println(buff);
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
    delayMicroseconds(120);
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
  sb.println(b, HEX);
  //    pinMode(DIO, INPUT_PULLUP);
  return b;
}

bool waitChange(int pin, int changeTo, int debugId) {
  uint32_t startMicros = micros();
  while (micros() - startMicros < 500) {
    if (digitalRead(pin) == changeTo)
      return true;
  }
  Serial.print("abort at ");
  Serial.println(debugId);
  return false;
}

