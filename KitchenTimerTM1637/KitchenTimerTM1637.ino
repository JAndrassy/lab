#include <TM1637Display.h> // author Avishay Orpaz
#include <Encoder.h> // author Paul Stoffregen
#include <Bounce2.h> // maintainer Thomas O Fredericks
#include <avr/sleep.h>

const byte BUTTON_PIN = 2;  // external interrupt pin
const byte ENCODER_PIN_A = 3; // external interrupt pin
const byte ENCODER_PIN_B = 4;
const byte DISPLAY_DATA_PIN = 5;
const byte DISPLAY_SCLK_PIN = 6;
const byte TONE_PIN = 9;
const byte LDR_PIN = A0;

const byte ENCODER_PULSES_PER_STEP = 2;
const unsigned DISPLAY_REFRESH_INTERVAL = 100; // milliseconds
const unsigned LONG_PUSH_INTERVAL = 3000; // miliseconds
const unsigned ALARM_BLINK_MILLIS = 700;
const unsigned TIMER_DISPLAY_TIMEOUT = 5000; // miliseconds
const unsigned ALARM_INTERVAL = 9600; // miliseconds

enum {
  SLEEP,
  SET_TIMER,
  COUNTDOWN,
  ALARM
};

TM1637Display display(DISPLAY_SCLK_PIN, DISPLAY_DATA_PIN);
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
Bounce button;

unsigned timerSeconds = 0;
byte displayData[4];

void setup() {

  button.attach(BUTTON_PIN, INPUT_PULLUP);

  // attachInterupt for wakeup with encoder button. alternative is to configure registers
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), [](){}, FALLING); // [](){} is empty anonymous function

  showTimer();
}

void loop() {

  static unsigned long previousMillis;
  static unsigned long displayRefreshMillis;
  static unsigned long displayTimeoutMillis;
  static unsigned long alarmStartMillis;
  static byte blink;
  static byte state = SET_TIMER;

  unsigned long currentMillis = millis();

  // countdown
  if (state == COUNTDOWN && currentMillis - previousMillis >= 1000) {
    previousMillis += 1000;
    if (timerSeconds > 0) {
      timerSeconds--;
      showTimer();
      if (timerSeconds == 0) {
        state = ALARM;
        blink = false;
        alarmStartMillis = millis();
        alarmSound(true); // with reset
      }
    }
  }

  // encoder
  int dir = encoder.read();
  if (abs(dir) >= ENCODER_PULSES_PER_STEP) {
    if (state == SLEEP) {
      state = SET_TIMER; // only wake-up
    } else {
      displayTimeoutMillis = 0; // reset timeout
      byte step;
      if (timerSeconds + dir > 6 * 60) {
        step = 60;
      } else if (timerSeconds + dir > 3 * 60) {
        step = 30;
      } else if (timerSeconds + dir > 60) {
        step = 15;
      } else if (timerSeconds == 0) {
        step = 10;
      } else {
        step = 5;
      }
      if (dir > 0) {
        if (state != COUNTDOWN) {
          step = step - (timerSeconds % step);
        }
        if (timerSeconds + step < 6000) { // 100 minutes
          timerSeconds += step;
          showTimer();
        }
      } else {
        if (state != COUNTDOWN) {
          int m = timerSeconds % step;
          if (m != 0) {
            step = m;
          }
        }
        if (timerSeconds >= step) {
          timerSeconds -= step;
          showTimer();
        }
      }
      if (state != COUNTDOWN) {
        state = SET_TIMER;
      }
    }
    encoder.write(0);
  }

  // button
  static unsigned long buttonPushedMillis;
  button.update();
  if (button.fell()) {
    buttonPushedMillis = currentMillis;
  }
  if (buttonPushedMillis && currentMillis - buttonPushedMillis > LONG_PUSH_INTERVAL) {
    buttonPushedMillis = 0;
    displayTimeoutMillis = 0;
    switch (state) {
      case SET_TIMER:
        timerSeconds = 0;
        showTimer();
        break;
    }
  }  
  if (button.rose() && buttonPushedMillis != 0) {
    buttonPushedMillis = 0;
    displayTimeoutMillis = 0;
    switch (state) {
      case SLEEP:
        state = SET_TIMER;
        break;
      case COUNTDOWN:
        state = SET_TIMER;
        break;
      case ALARM:
        state = SET_TIMER;
        showTimer();
        break;
      case SET_TIMER:
        if (timerSeconds > 0) {
          state = COUNTDOWN;
          previousMillis = millis();
        }
        break;
    }
  }

  // timer display timeout
  if (state == SET_TIMER) {
    if (displayTimeoutMillis == 0) {
      displayTimeoutMillis = currentMillis;
    }
    if ((currentMillis - displayTimeoutMillis > TIMER_DISPLAY_TIMEOUT)) {
      displayTimeoutMillis = 0;
      state = SLEEP;
      display.setBrightness(0, false);
      display.showNumberDec(0, 3, 0);
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      sleep_enable();
      sleep_cpu();
    }
  }

  // alarm
  if (state == ALARM) {
    if (currentMillis - previousMillis >= ALARM_BLINK_MILLIS) {
      previousMillis = currentMillis;
      blink = !blink;
    }
    alarmSound(false);
    if (currentMillis - alarmStartMillis > ALARM_INTERVAL) {
      state = SET_TIMER;
    }
  }

  // display refresh
  if (currentMillis - displayRefreshMillis > DISPLAY_REFRESH_INTERVAL) {
    displayRefreshMillis = currentMillis;
    static int lastLDRReading = 1300; // init out of range
    if (state == ALARM) {
      display.setBrightness(7, blink);
      lastLDRReading = 1300;
    } else {
      int a = analogRead(LDR_PIN);
      if (abs(a - lastLDRReading) > 20) {
        lastLDRReading = a;
        byte brightness = map(a, 0, 1024, 1, 8);
        display.setBrightness(brightness, true);
      }
    }
    display.setSegments(displayData);
  }
}

void showTimer() {

  byte minutes = timerSeconds / 60;
  byte secs = timerSeconds % 60;

  byte digitBuffer[4];
  digitBuffer[0] = minutes / 10;
  digitBuffer[1] = minutes % 10;
  digitBuffer[2] = secs / 10;
  digitBuffer[3] = secs % 10;

  for (int i = 0; i < 4; i++) {
    displayData[i] = display.encodeDigit(digitBuffer[i]);
  }
  displayData[1] |= 0x80; // colon
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
