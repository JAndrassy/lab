/**
 * avrdude running on Linux can use telnet to do a serial upload.
 * this esp8266 sketch receives the upload and forwards it to Serial.
 * On the other side of the esp8266 Serial should be a Serial port
 * of AVR MCU with bootloader (the RX/TX pins where the bootloader listens)
 *
 * (the SERIAL_SWITCH_PIN is for Uno WiFi Dev Ed, where the Serial connection
 * is switched by the esp between esp and the USB chip.)
 *
 * example of the upload command:
 * avrdude -Cavrdude.conf -v -patmega328p -carduino -Pnet:192.168.1.102:23 -D -Uflash:w:Blink.ino.hex:i
 *
 *  created in December 2019
 *  by Juraj Andrassy https://github.com/jandrassy
 */

#include <ESP8266WiFi.h>

//const int SERIAL_SWITCH_PIN = 4;
const int TARGET_RESET_PIN = 0;

WiFiServer telnetServer(23);

void setup() {

  Serial.begin(115200);
  delay(100);
  Serial.println("START");

#ifdef SERIAL_SWITCH_PIN
  pinMode(SERIAL_SWITCH_PIN, OUTPUT);
  digitalWrite(SERIAL_SWITCH_PIN, HIGH);
#endif

  if (!WiFi.waitForConnectResult()) {
    Serial.println("WiFi not connected");
    while (1) {
      delay(1000);
    }
  }

  Serial.println(WiFi.localIP());

  telnetServer.begin();

}

void loop() {

  WiFiClient telnetClient  = telnetServer.available();
  if (telnetClient) {

#ifdef SERIAL_SWITCH_PIN
    digitalWrite(SERIAL_SWITCH_PIN, LOW);
#endif

    pinMode(TARGET_RESET_PIN, OUTPUT);
    digitalWrite(TARGET_RESET_PIN, LOW);
    delay(1);
    pinMode(TARGET_RESET_PIN, INPUT); // let it to reset pin's pull-up

    while (telnetClient.connected() || telnetClient.available()) {
      while (telnetClient.available()) {
        Serial.write(telnetClient.read());
      }
      while (Serial.available()) {
        telnetClient.write(Serial.read());
      }
    }
#ifdef SERIAL_SWITCH_PIN
    digitalWrite(SERIAL_SWITCH_PIN, HIGH);
#endif
  }
}
