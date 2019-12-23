
//#define FLASHING // uncomment for use with esptool and FDT

#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include <NeoSWSerial.h>
NeoSWSerial Serial1(6, 7);
#define BAUD 38400
#else
#define BAUD 115200
#endif

#define ESP_RESET_PIN 9
#define ESP_GPIO0_PIN 8

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(BAUD);
  Serial1.begin(BAUD);

#ifdef FLASHING
  digitalWrite(LED_BUILTIN, HIGH);
  resetESP(true); // reset to bootloader. no need to push the B/L button while connecting to USB
#else
  digitalWrite(LED_BUILTIN, LOW);
  resetESP(false); // power cycle
#endif
}

void loop() {
  while (Serial.available()) {
    detectFlashing();
    Serial1.write(Serial.read());
  }
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  detectFlashingEnd();
}

#ifdef FLASHING
void detectFlashing() {
  // empty
}
void detectFlashingEnd() {
  // empty
}
#else
const byte syncFrame[] = {0xC0, 0x00, 0x08, 0x24, 0x00, 0xDD, 0x00, 0x00, 0x00, 0x07, 0x07, 0x12, 0x20};
const byte checkSumPos = 5; // esptool.py and FDT do not send the checksum for sync frame, IDE does
byte syncFrameIndex = 0;

void detectFlashing() {
  if (syncFrameIndex == sizeof(syncFrame))
    return;
  byte b = Serial.peek();
  if (!(b == syncFrame[syncFrameIndex] || (syncFrameIndex == checkSumPos && b == 0x00))) {
    syncFrameIndex = 0;
  } else {
    syncFrameIndex++;
    if (syncFrameIndex == sizeof(syncFrame)) {
      resetESP(true); // reset to bootloader
      digitalWrite(LED_BUILTIN, HIGH);
      Serial1.write(syncFrame, sizeof(syncFrame) - 1); // last byte was not read, only peek
    }
  }
}

void detectFlashingEnd() {
  if (syncFrameIndex < sizeof(syncFrame))
    return;
  static unsigned long lastFlashingActivity;
  if (Serial.available() || !lastFlashingActivity) {
    lastFlashingActivity = millis();
  } else if (millis() - lastFlashingActivity > 5000) { // wait so much. perhaps the tool makes the reset
    digitalWrite(LED_BUILTIN, LOW);
    resetESP(false); // power cycle ESP after flashing, to avoid GPIO_STRAPPING register ESP bug
    syncFrameIndex = 0;
    lastFlashingActivity = 0;
  }
}
#endif

void resetESP(boolean toBootloader) {
  if (toBootloader) {
    pinMode(ESP_GPIO0_PIN, OUTPUT);
    digitalWrite(ESP_GPIO0_PIN, LOW);
  }
  pinMode(ESP_RESET_PIN, OUTPUT);
  delay(1);
  pinMode(ESP_RESET_PIN, INPUT); // let it to pull-up resistor
  if (toBootloader) {
    delay(50);
    pinMode(ESP_GPIO0_PIN, INPUT); // let it to pull-up resistor
  }
}
