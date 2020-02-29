#include <TM1637Display.h>
#include <Encoder.h>
#include <Bounce2.h>

const byte BUTTON_PIN = 2;  // external interrupt pin
const byte ENCODER_PIN_A = 3; // external interrupt pin
const byte ENCODER_PIN_B = 4;
const byte DISPLAY_SCLK_PIN = 5;
const byte DISPLAY_DATA_PIN = 6;
const byte TONE_PIN = 9;
const byte LDR_PIN = A0;

const unsigned long LONG_PUSH_INTERVAL = 3000; // miliseconds
const unsigned long SET_TIME_BLINK_MILLIS = 250;
const unsigned long ALARM_SOUND_INTERVAL = 5500; // miliseconds
const unsigned long ALARM_SOUND_REPEAT_INTERVAL = 60000; // miliseconds
const unsigned long TIMER_DISPLAY_TIMEOUT = 60000; // miliseconds
const unsigned long MINUTE_MILIS = 60000; // to fine tune clock

enum {
  CLOCK,
  SET_HOUR,
  SET_MINUTE,
  SET_TIMER,
  COUNTDOWN,
  ALARM
};

TM1637Display display(DISPLAY_SCLK_PIN, DISPLAY_DATA_PIN);
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
Bounce button;

uint8_t clockHour = 0;
uint8_t clockMinute = 0;
uint16_t timerSeconds = 0;
byte digitBuffer[4];

int lastLDRReading;

void setup() {
  button.attach(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {

  static unsigned long previousMillis;
  static unsigned long displayTimeoutMillis;
  static unsigned long alarmStartMillis;
  static unsigned long timerStartSeconds;
  static unsigned long minuteMillis;
  static byte blink;
  static byte state = CLOCK;

  unsigned long currentMillis = millis();

  // clock
  if (state == SET_HOUR || state == SET_MINUTE) {
    if (currentMillis - previousMillis >= SET_TIME_BLINK_MILLIS) {
      previousMillis = currentMillis;
      blink = !blink;
      if (state == SET_HOUR) {
        showClock(true, blink, true);
      } else {
        showClock(true, true, blink);
      }
    }
  } else if (currentMillis - minuteMillis > MINUTE_MILIS) {
    minuteMillis += MINUTE_MILIS;
    if (clockMinute < 59) {
      clockMinute++;
    } else {
      clockMinute = 0;
      clockHour = (clockHour < 23) ? (clockHour + 1) : 0;
    }
  }
  if (state == CLOCK && currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    blink = !blink;
    showClock(blink, true, true);
  }

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
  static unsigned long encoderDebounceMillis;
  int dir = encoder.read();
  if (dir) {
    if (currentMillis - encoderDebounceMillis >= 50) {
      encoderDebounceMillis = currentMillis;
      if (state == SET_HOUR) {
        if (dir > 0) {
          clockHour = (clockHour < 23) ? (clockHour + 1) : 0;
        } else {
          clockHour = (clockHour > 0) ? (clockHour - 1) : 23;
        }
      } else if (state == SET_MINUTE) {
        if (dir > 0) {
          clockMinute = (clockMinute < 59) ? (clockMinute + 1) : 0;
        } else {
          clockMinute = (clockMinute > 0) ? (clockMinute - 1) : 59;
        }
      } else {
        displayTimeoutMillis = 0; // reset timeout
        int step;
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
          if (timerSeconds < 6000) { // 100 minutes
          if (state != COUNTDOWN) {
            step = step - (timerSeconds % step);
          }
            timerSeconds += step;
            showTimer();
          }
        } else if (timerSeconds > 0) {
        if (state != COUNTDOWN) {
          int m = timerSeconds % step;
          if (m != 0) {
            step = m;
          }
        }
          timerSeconds -= step;
          showTimer();
        }
        if (state != COUNTDOWN) {
          state = SET_TIMER;
        }
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
        timerStartSeconds = 0;
        showTimer();
        break;
      case CLOCK:
        state = SET_HOUR;
        break;
    }
  }
  if (button.rose() && buttonPushedMillis != 0) {
    buttonPushedMillis = 0;
    displayTimeoutMillis = 0;
    switch (state) {
      case SET_HOUR:
        state = SET_MINUTE;
        break;
      case SET_MINUTE:
        state = CLOCK;
        minuteMillis = millis();
        break;
      case CLOCK:
        state = SET_TIMER;
        showTimer();
        break;
      case COUNTDOWN:
        state = SET_TIMER;
        break;
      case ALARM:
        state = SET_TIMER;
        timerSeconds = timerStartSeconds;
        lastLDRReading = 0; // to reset full brightness of ALARM
        showTimer();
        break;
      case SET_TIMER:
        if (timerSeconds > 0) {
          state = COUNTDOWN;
          previousMillis = millis();
          if (timerStartSeconds < timerSeconds) {
            timerStartSeconds = timerSeconds;
          }
        } else {
          state = CLOCK;
          timerStartSeconds = 0;
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
      state = CLOCK;
      timerSeconds = 0;
      timerStartSeconds = 0;
    }
  }

  // alarm
  if (state == ALARM) {
    if (currentMillis - previousMillis >= 700) {
      previousMillis = currentMillis;
      blink = !blink;
      display.setBrightness(7, blink);
      display.showNumberDecEx(0, 0b0100000, 3, 0);
    }
    if (currentMillis - alarmStartMillis < ALARM_SOUND_INTERVAL) {
      alarmSound(false);
    }
    if (currentMillis - alarmStartMillis > ALARM_SOUND_REPEAT_INTERVAL) {
      alarmStartMillis = currentMillis;
    }
  }
}

void showClock(bool showColon, bool showHour, bool showMinute) {

  digitBuffer[0] = clockHour / 10;
  digitBuffer[1] = clockHour % 10;
  digitBuffer[2] = clockMinute / 10;
  digitBuffer[3] = clockMinute % 10;

  refreshDisplay(showColon, showHour, showMinute);
}

void showTimer() {

  uint8_t minutes = timerSeconds / 60;
  uint8_t secs = timerSeconds % 60;

  digitBuffer[0] = minutes / 10;
  digitBuffer[1] = minutes % 10;
  digitBuffer[2] = secs / 10;
  digitBuffer[3] = secs % 10;

  refreshDisplay(true, true, true);
}

void refreshDisplay(bool showColon, bool showLeft, bool showRight) {
  uint8_t data[4] = {0, 0, 0, 0};
  if (showLeft) {
    data[0] = display.encodeDigit(digitBuffer[0]);
    data[1] = display.encodeDigit(digitBuffer[1]);
  }
  if (showRight) {
    data[2] = display.encodeDigit(digitBuffer[2]);
    data[3] = display.encodeDigit(digitBuffer[3]);
  }
  if (showColon) {
    data[1] |= 0x80; // colon
  }

  int a = analogRead(LDR_PIN);
  if (abs(a - lastLDRReading) > 10) {
    lastLDRReading = a;
    byte brightness = map(a, 0, 1024, 0, 8);
    display.setBrightness(brightness, true);
  }
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
