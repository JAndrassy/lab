

#if !defined(HAVE_HWSERIAL1)
#include <NeoSWSerial.h>
NeoSWSerial Serial1(6, 7);
#define BAUD 38400
#else
#define BAUD 115200
#endif

#define ESP_ENABLE_PIN A1
#define ESP_GPIO0_PIN A0

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(BAUD);
  Serial1.begin(BAUD);

  resetESP(false); // to bridge the reset
  detectEspFlashing();
}

void loop() {
  while (Serial.available()) {
    detectEspFlashing();
    Serial1.write(Serial.read());
  }
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}

void detectEspFlashing() {
  const byte syncFrame[] = {0xC0, 0x00, 0x08, 0x24, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x07, 0x07, 0x12, 0x20};
  byte syncFrameIndex = 0;
  while (millis() < 2000) { // 2 seconds after reset
    if (!Serial.available())
      continue;
    int b = Serial.read();
    if (b == -1)
      return;
    if (b != syncFrame[syncFrameIndex]) {
      syncFrameIndex = 0;
    } else {
      syncFrameIndex++;
      if (syncFrameIndex == sizeof(syncFrame)) {
        resetESP(true); // reset to bootloader
        Serial1.write(syncFrame, sizeof(syncFrame));
        while (true) {
          while (Serial.available()) {
            Serial1.write(Serial.read());
          }
          while (Serial1.available()) {
            Serial.write(Serial1.read());
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
