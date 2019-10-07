
#if !defined(HAVE_HWSERIAL3)
#include <NeoSWSerial.h>
NeoSWSerial Serial3(6, 7);
#define BAUD 38400
#else
#define BAUD 115200
#endif

#define ESP_ENABLE_PIN 2
#define ESP_GPIO0_PIN 3

void setup() {
  Serial.begin(BAUD);
  Serial3.begin(BAUD);

  resetESP(false); // to bridge the reset
  detectEspFlashing();
}

void loop() {

  static unsigned long next = millis();
  if (millis() > next) {
    next += 5000;

    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:

    Serial.println(voltage);

    // and here send the value to your esp sketch over Serial3

  }
}

void detectEspFlashing() {
  const byte syncFrame[] = {0xC0, 0x00, 0x08, 0x24, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x07, 0x07, 0x12, 0x20};
  byte syncFrameIndex = 0;
  while (millis() < 2000) { // 2 seconds after reset
    if (!Serial.available())
      continue;
    byte b = Serial.read();
    if (b == -1)
      return;
    if (b != syncFrame[syncFrameIndex]) {
      syncFrameIndex = 0;
    } else {
      syncFrameIndex++;
      if (syncFrameIndex == sizeof(syncFrame)) {
        resetESP(true); // reset to bootloader
        Serial3.write(syncFrame, sizeof(syncFrame));
        while (true) {
          while (Serial.available()) {
            Serial3.write(Serial.read());
          }
          while (Serial3.available()) {
            Serial.write(Serial3.read());
          }
        }
      }
    }
  }
}

void resetESP(boolean toBootloader) {
  if (toBootloader) {
    pinMode(ESP_GPIO0_PIN, OUTPUT);
    digitalWrite(ESP_GPIO0_PIN, LOW);
  }
  digitalWrite(ESP_ENABLE_PIN, LOW);
  delay(5);
  pinMode(ESP_ENABLE_PIN, INPUT); // let it to pull-up resistor
  if (toBootloader) {
    delay(100);
    pinMode(ESP_GPIO0_PIN, INPUT); // let it to pull-up resistor
  }
}
