#include <Encoder.h>
#include <avr/sleep.h>

const byte BUTTON_PIN = 2;  // external interrupt pin
const byte ENCODER_PIN_A = 3; // external interrupt pin
const byte ENCODER_PIN_B = 4;
const byte DISPLAY_DATA_PIN = 5;
const byte DISPLAY_RCLK_PIN = 6;
const byte DISPLAY_SCLK_PIN = 7;
const byte TONE_PIN = 9;
const byte LDR_PIN = A0;

const unsigned long DISPLAY_REFRESH_INTERVAL_MAX = 10000; // microseconds (power saving)
const unsigned long LDR_ANALOG_READ_MULTIPLE = (DISPLAY_REFRESH_INTERVAL_MAX / 1000) - 1; // 1000 because max analog reading is 1023
const unsigned long DISPLAY_TIMEOUT = 5000; // miliseconds
const unsigned long ALARM_INTERVAL = 10000; // miliseconds

enum {
  PAUSE,
  COUNTDOWN,
  ALARM
};

Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

uint16_t seconds = 0;
byte digitBuffer[4] = {0, 0, 0, 0};

void setup() {
  pinMode(DISPLAY_DATA_PIN, OUTPUT);
  pinMode(DISPLAY_RCLK_PIN, OUTPUT);
  pinMode(DISPLAY_SCLK_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // attachInterupt for wakeup with encoder button. alternative is to configure registers
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), [](){}, FALLING); // [](){} is empty anonymous function

  showDisplay(); // to clear it
}

void loop() {

  static unsigned long previousMillis;
  static unsigned long encoderDebounceMillis;
  static unsigned long buttonDebounceMillis;
  static unsigned long displayRefreshMicros;
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
      seconds2digits();
      if (seconds == 0) {
        state = ALARM;
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
          seconds2digits();
        }
      } else if (seconds > 0) {
        seconds -= step;
        seconds2digits();
      }
      state = PAUSE;
    }
    encoder.write(0);
  }

  // button
  if (digitalRead(BUTTON_PIN) == LOW && currentMillis - buttonDebounceMillis >= 500) {
    buttonDebounceMillis = currentMillis;
    state = (state == PAUSE) ? COUNTDOWN : PAUSE;
  }

  // display timeout and sleep
  if (state == PAUSE) {
    if (displayTimeoutMillis == 0) {
      displayTimeoutMillis = currentMillis;
    }
    if ((currentMillis - displayTimeoutMillis > DISPLAY_TIMEOUT)) {
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      sleep_enable();
      sleep_cpu();
    }
  }

  unsigned long displayRefreshInterval = DISPLAY_REFRESH_INTERVAL_MAX - (analogRead(LDR_PIN) * LDR_ANALOG_READ_MULTIPLE);

  // display refresh
  if (state != ALARM && micros() - displayRefreshMicros > displayRefreshInterval) {
    displayRefreshMicros = micros();
    showDisplay();
  }

  // alarm
  if (state == ALARM) {
    if (currentMillis - previousMillis >= 500) {
      previousMillis = millis();
      alarmBlink = !alarmBlink;
    }
    if (alarmBlink) {
      showDisplay(); // refreshed in every loop for full brightness
    }
    alarmSound(false);
    if (millis() - alarmStartMillis > ALARM_INTERVAL) {
      state = PAUSE;
    }
  }

}

void seconds2digits() {
  uint8_t minutes = seconds / 60;
  uint8_t secs = seconds % 60;

  digitBuffer[0] = minutes / 10;
  digitBuffer[1] = minutes % 10;
  digitBuffer[2] = secs / 10;
  digitBuffer[3] = secs % 10;
}

void showDisplay() {

  const byte digit[10] = { //
      0b11000000, // 0
      0b11111001, // 1
      0b10100100, // 2
      0b10110000, // 3
      0b10011001, // 4
      0b10010010, // 5
      0b10000010, // 6
      0b11111000, // 7
      0b10000000, // 8
      0b10010000, // 9
      };

  const byte chr[4] = { 0b00001000, 0b00000100, 0b00000010, 0b00000001 };

  for (byte i = 0; i < 4; i++) {
    digitalWrite(DISPLAY_RCLK_PIN, LOW);
    shiftOut(DISPLAY_DATA_PIN, DISPLAY_SCLK_PIN, MSBFIRST, digit[digitBuffer[i]]);
    shiftOut(DISPLAY_DATA_PIN, DISPLAY_SCLK_PIN, MSBFIRST, chr[i]);
    digitalWrite(DISPLAY_RCLK_PIN, HIGH);
  }
  digitalWrite(DISPLAY_RCLK_PIN, LOW);
  shiftOut(DISPLAY_DATA_PIN, DISPLAY_SCLK_PIN, MSBFIRST, digit[0]);
  shiftOut(DISPLAY_DATA_PIN, DISPLAY_SCLK_PIN, MSBFIRST, 0);
  digitalWrite(DISPLAY_RCLK_PIN, HIGH);
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
