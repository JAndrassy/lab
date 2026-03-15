#include <ESP8266WiFi.h>
#include <TZ.h>  // in esp8266 core
#include <coredecls.h> // for settimeofday_cb()
#include <Bounce2.h> // maintainer Thomas O Fredericks
using namespace Bounce2;

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password

#define TIME_ZONE TZ_Europe_Bratislava
const uint8_t INDICATOR_LED_PIN = D4; // built-in LED of Wemos D1
const uint8_t RELAY_PIN = D1;
const uint8_t BUTTON_PIN = D2;

const int ON_MINUTE = 6 * 60 + 0;
const int OFF_MINUTE = 22 * 60 + 0;
bool timerRelayOn;
bool buttonRelayOn;

Button button;

void setup() {

  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  timerRelayOn = true;

  pinMode(INDICATOR_LED_PIN, OUTPUT);
  digitalWrite(INDICATOR_LED_PIN, LOW); // on (inverted circuit)

  button.attach(BUTTON_PIN, INPUT_PULLUP);
  button.setPressedState(LOW);
  buttonRelayOn = timerRelayOn;

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
     WiFi.begin(ssid, pass);
  }
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Not connected");
  } else {
    Serial.println(WiFi.localIP());
  }
  WiFi.enableAP(false);
  WiFi.persistent(false); // to not store mode changes}

  settimeofday_cb([]() {
    Serial.println("got time");
    WiFi.mode(WIFI_OFF);
    digitalWrite(INDICATOR_LED_PIN, HIGH); // off (inverted circuit)
    checkRelayState();
  });
  configTime(TIME_ZONE, "pool.ntp.org");
}

void loop() {

  static unsigned long ntpPrevMillis;
  static unsigned long checkPrevMillis;

  unsigned long currentMillis = millis();

  // WiFi for NTP query
  if ((currentMillis - ntpPrevMillis > (1000L * 60 * 60 * 24) || !ntpPrevMillis)) {
    ntpPrevMillis = currentMillis;
    WiFi.begin();
  }
  if (WiFi.isConnected() && currentMillis - ntpPrevMillis > 30000) {
    Serial.println("sntp timeout");
    WiFi.mode(WIFI_OFF);
    digitalWrite(INDICATOR_LED_PIN, HIGH); // off (inverted circuit)
  }

  if (currentMillis - checkPrevMillis > 60000) {
    checkPrevMillis = currentMillis;
    checkRelayState();
  }

  button.update();
  if (button.released()) {
    buttonRelayOn = !buttonRelayOn;
    digitalWrite(RELAY_PIN, buttonRelayOn ? LOW : HIGH); // using NC terminal of the relay
    Serial.println(buttonRelayOn);
  }
}

void checkRelayState() {
  time_t t = time(nullptr);
  struct tm *tm = localtime(&t);
  if (tm->tm_year >= 120) {
    int hour = tm->tm_hour;
    int minute = tm->tm_min;
    Serial.print(hour);
    Serial.print(':');
    Serial.println(minute);
    int dayMinutes = hour * 60 + minute;
    if (!timerRelayOn && dayMinutes >= ON_MINUTE  && dayMinutes < OFF_MINUTE) {
      digitalWrite(RELAY_PIN, LOW); // using NC terminal of the relay
      timerRelayOn = true;
      buttonRelayOn = timerRelayOn;
      Serial.println("ON");
    }
    if (timerRelayOn && (dayMinutes < ON_MINUTE || dayMinutes >= OFF_MINUTE)) {
      digitalWrite(RELAY_PIN, HIGH); // using NC terminal of the relay
      timerRelayOn = false;
      buttonRelayOn = timerRelayOn;
      Serial.println("OFF");
    }
  }
}