#include <TM1637Display.h>
#include <Encoder.h>
#include <avr/sleep.h>

const byte BUTTON_PIN = 2;  // external interrupt pin
const byte ENCODER_PIN_A = 3; // external interrupt pin
const byte ENCODER_PIN_B = 4;
const byte DISPLAY_SCLK_PIN = 5;
const byte DISPLAY_DATA_PIN = 6;
const byte TONE_PIN = 9;
const byte LDR_PIN = A0;

const unsigned long DISPLAY_TIMEOUT = 5000; // miliseconds
const unsigned long ALARM_INTERVAL = 10000; // miliseconds

enum {
  PAUSE,
  COUNTDOWN,
  ALARM
};

TM1637Display display(DISPLAY_SCLK_PIN, DISPLAY_DATA_PIN);
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

uint16_t seconds = 0;
byte digitBuffer[4];

void setup() {
  pinMode(DISPLAY_DATA_PIN, OUTPUT);
  pinMode(DISPLAY_SCLK_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // attachInterupt for wakeup with encoder button. alternative is to configure registers
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), [](){}, FALLING); // [](){} is empty anonymous function

  showDigits();
}

void loop() {

  static unsigned long previousMillis;
  static unsigned long encoderDebounceMillis;
  static unsigned long buttonDebounceMillis;
  static unsigned long displayTimeoutMillis;
  static unsigned long alarmStartMillis;
  static byte alarmBlink;
  static byte state = PAUSE;

  unsigned long currentMillis = millis();

  // countdown
  if (state == COUNTDOWN && currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    if (seconds > 0) {
      seconds--;
      showDigits();
      if (seconds == 0) {
        state = ALARM;
        alarmBlink = false;
        alarmStartMillis = millis();
        alarmSound(true); // with reset
      }
    }
  }

  // encoder
  int dir = encoder.read();
  if (dir) {
    if (currentMillis - encoderDebounceMillis >= 50) {
      encoderDebounceMillis = currentMillis;
      displayTimeoutMillis = 0; // reset timeout
      int step = seconds < 60 ? 1 : 10;
      if (dir > 0) {
        if (seconds < 6000) { // 100 minutes
          seconds += step;
          showDigits();
        }
      } else if (seconds > 0) {
        seconds -= step;
        showDigits();
      }
      state = PAUSE;
    }
    encoder.write(0);
  }

  // button
  if (digitalRead(BUTTON_PIN) == LOW && currentMillis - buttonDebounceMillis >= 500) {
    buttonDebounceMillis = currentMillis;
    state = (state == PAUSE) ? COUNTDOWN : PAUSE;
    displayTimeoutMillis = 0;
  }

  // display timeout and sleep
  if (state == PAUSE) {
    if (displayTimeoutMillis == 0) {
      displayTimeoutMillis = currentMillis;
    }
    if ((currentMillis - displayTimeoutMillis > DISPLAY_TIMEOUT)) {
      noTone(TONE_PIN);
      display.setBrightness(0, false);
      display.showNumberDec(0, 3, 0);
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      sleep_enable();
      sleep_cpu();
    }
  }

  // alarm
  if (state == ALARM) {
    if (currentMillis - previousMillis >= 700) {
      previousMillis = millis();
      alarmBlink = !alarmBlink;
      display.setBrightness(7, alarmBlink);
      display.showNumberDecEx(0, 0b0100000, 3, 0);
    }
    alarmSound(false);
    if (millis() - alarmStartMillis > ALARM_INTERVAL) {
      state = PAUSE;
    }
  }

}

void showDigits() {

  uint8_t minutes = seconds / 60;
  uint8_t secs = seconds % 60;

  digitBuffer[0] = minutes / 10;
  digitBuffer[1] = minutes % 10;
  digitBuffer[2] = secs / 10;
  digitBuffer[3] = secs % 10;

  uint8_t data[4];
  for (int i = 0; i < 4; i++) {
    data[i] = display.encodeDigit(digitBuffer[i]);
  }
  data[1] |= 0x80; // colon

  byte brightness = map(analogRead(LDR_PIN), 0, 1024, 1, 8);
  display.setBrightness(brightness);
  display.setSegments(data);
}

void alarmSound(bool reset) {
  const int ALARM_BEEP_1 = 4186;
  const int ALARM_BEEP_2 = 4699;

  const int ALARM_TONE_LENGTH = 200;
  const int ALARM_TONE_PAUSE = 800;
  const int ALARM_TONE_REPEAT = 6;

  static unsigned long next = millis();
  static byte count = 0;

  if (reset) {
    count = 0;
    next = millis();
  }

  if (millis() > next) {
    next += ALARM_TONE_LENGTH;
    count++;
    if (count == ALARM_TONE_REPEAT) {
      next += ALARM_TONE_PAUSE;
      count = 0;
    }
    tone(TONE_PIN, (count % 2) ? ALARM_BEEP_1 : ALARM_BEEP_2, ALARM_TONE_LENGTH);
  }
}
