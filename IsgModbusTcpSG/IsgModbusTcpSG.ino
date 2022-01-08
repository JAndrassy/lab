/*
  This Arduino sketch is for DIY "SG Ready" adapter for Stiebel Eltron ISGweb.
  The ISGweb device is Internet Service Gateway for Stiebel Eltron heat pumps.
  The ISGweb doesn't have physical SG Ready input pins, but SG Ready inputs
  can be be controlled over Modbus TCP.

  This sketch reads a state of digital input input pin of Arduino and sets
  the state over Modbus TCP to SG Ready INPUT1 register of ISGweb.

  Network shield, module or on board TCP/IP networking capability is required.
  Ethernet library for Wiznet W5x00 chips or UIPEthernet library for enc28j60.
  It is easy to adapt the sketch for one of WiFi libraries for Arduino.

  To access the adapter over network point telnet client to the IP of the adapter
  and port 2323.

  Copyright 2020 Juraj Andrassy https://github.com/jandrassy

  This sketch is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <EthernetENC.h> // or <Ethernet.h>

#define VERSION "0.17"


const byte FNC_H_READ_REGS = 0x03;
const byte FNC_I_READ_REGS = 0x04;
const byte FNC_WRITE_SINGLE = 0x06;
const byte FNC_ERR_FLAG = 0x80;
const int MODBUS_NO_RESPONSE = -11;

const byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const byte INPUT_PIN = 3;
const byte ETH_CS_PIN = 10;

const byte CHECK_OP_STATE_INTERVAL = 15; // seconds

const IPAddress ip(192, 168, 1, 200);
const IPAddress isgAddress(192, 168, 1, 100);

EthernetServer telnetServer(2323);
EthernetClient telnetClient;
Stream* terminal;

bool automaticMode = true;
bool isgSgInput1IsON;
bool waitingForSGOpStateChange = false;

void setup() {
  Serial.begin(115200);
  terminal = &Serial;

  pinMode(INPUT_PIN, INPUT_PULLUP);

  Serial.println(F("ISGweb SG Ready adapter version " VERSION));

  Ethernet.init(ETH_CS_PIN);
  Ethernet.begin(mac, ip);

  telnetServer.begin();

  if (automaticMode) {
    switchIsgSgInput1(inputPinIsON());
    printState();
  }
}

void loop() {
  Ethernet.maintain();

  if (!telnetClient) {
    telnetClient = telnetServer.accept();
    if (telnetClient.connected()) {
      telnetClient.println(F("ISGweb SG Ready adapter version " VERSION));
      terminal = &telnetClient;
    } else {
      terminal = &Serial;
    }
  }

  if (automaticMode) {
    bool on = inputPinIsON();
    if (isgSgInput1IsON != on) {
      terminal->print(F("Signal changed to "));
      terminal->println(on ? "ON" : "OFF");
      switchIsgSgInput1(on);
    }
  }

  int ch = terminal->read();
  switch (ch) {
    case 'P':
      printState();
      break;
    case 'A':
      if (automaticMode) {
        terminal->println(F("Already in automatic mode"));
      } else {
        automaticMode = true;
        terminal->println(F("Automatic mode activated"));
        switchIsgSgInput1(inputPinIsON());
        printState();
      }
      break;
    case '0':
    case '1':
      if (automaticMode) {
        automaticMode = false;
        terminal->println(F("Manual mode activated"));
      }
      switchIsgSgInput1(ch - '0');
      printState();
      break;
    case 'C':
      if (telnetClient.connected()) {
        telnetClient.stop();
      }
      break;
  }

  static unsigned long checkSGOpStateMillis = millis();

  if (waitingForSGOpStateChange && millis() - checkSGOpStateMillis > (1000UL * CHECK_OP_STATE_INTERVAL)) {
    checkSGOpState();
    checkSGOpStateMillis = millis();
  }

}

bool inputPinIsON() {
  return (digitalRead(INPUT_PIN) == LOW);
}

void switchIsgSgInput1(bool on) {

  terminal->print(F("Setting SG INPUT1 register to "));
  terminal->println(on ? "ON" : "OFF");

  EthernetClient client;
  if (!client.connect(isgAddress, 502)) {
    terminal->println(F("Error: connection failed"));
    return;
  }
  int res = modbusWriteSingle(client, 4001, on);
  client.stop();
  if (res != 0) {
    terminal->print(F("Error setting SG INPUT1 register. error code: "));
    terminal->println(res);
    delay(1000);
    return;
  }
  isgSgInput1IsON = on;
  waitingForSGOpStateChange = true;
}

void checkSGOpState() {
  EthernetClient client;
  if (!client.connect(isgAddress, 502)) {
    terminal->println(F("Error: connection failed"));
    return;
  }
  short regs[1];
  int res = modbusRequest(client, FNC_I_READ_REGS, 5000, 1, regs);
  client.stop();
  if (res != 0) {
    terminal->print(F("Error reading register 5001, error code: "));
    terminal->println(res);
    return;
  }
  terminal->print(F("Register 5001 (SG READY OPERATING STATE): "));
  terminal->println(regs[0]);
  if (regs[0] == (isgSgInput1IsON ? 3 : 2)) { // operating states 2 and 3
    waitingForSGOpStateChange = false;
    terminal->print(F("Operating state changed to "));
    terminal->println(regs[0]);
  }
}

void printState() {

  terminal->println();
  terminal->println(automaticMode ? F("Automatic mode") : F("Manual mode"));

  terminal->print(F("Last SG INPUT1 command was "));
  terminal->println(isgSgInput1IsON ? "ON" : "OFF");

  bool inputPinIsON = (digitalRead(INPUT_PIN) == LOW);
  terminal->print(F("Input pin state is "));
  terminal->println(inputPinIsON ? "ON" : "OFF");

  EthernetClient client;

  if (!client.connect(isgAddress, 502)) {
    terminal->println(F("Error: connection failed"));
    return;
  }

  short regs[3] = {0, 0, 0};

  int res = modbusRequest(client, FNC_H_READ_REGS, 4000, 3, regs);
  if (res != 0) {
    terminal->print(F("modbus error "));
    terminal->println(res);
  } else {
//      terminal->print(F("Register 4001 (SG READY ON/OFF): "));
//      terminal->println(regs[0]);
    terminal->print(F("Register 4002 (SG READY INPUT 1): "));
    terminal->println(regs[1]);
//      terminal->print(F("Register 4003 (SG READY INPUT 2): "));
//      terminal->println(regs[2]);
  }

  res = modbusRequest(client, FNC_I_READ_REGS, 5000, 1, regs);
  client.stop();
  if (res != 0) {
    terminal->print(F("modbus error "));
    terminal->println(res);
  } else {
    terminal->print(F("Register 5001 (SG READY OPERATING STATE): "));
    terminal->println(regs[0]);
  }
  terminal->println();
}

/*
 * return
 *   - 0 is success
 *   - negative is comm error
 *   - positive value is client protocol exception code
 */
int modbusRequest(Client& client, byte fnc, unsigned int addr, byte len, short *regs) {

  const byte CODE_IX = 7;
  const byte ERR_CODE_IX = 8;
  const byte LENGTH_IX = 8;
  const byte DATA_IX = 9;

  client.setTimeout(2000);

  byte request[] = {0, 1, 0, 0, 0, 6, 1, fnc, (byte) (addr / 256), (byte) (addr % 256), 0, len};
  client.write(request, sizeof(request));

  int respDataLen = len * 2;
  byte response[max((int) DATA_IX, respDataLen)];
  int readLen = client.readBytes(response, DATA_IX);
  if (readLen < DATA_IX)
    return MODBUS_NO_RESPONSE;
  if (response[CODE_IX] == (FNC_ERR_FLAG | fnc))
    return response[ERR_CODE_IX]; // 0x01, 0x02, 0x03 or 0x11
  if (response[CODE_IX] != fnc)
    return -2;
  if (response[LENGTH_IX] != respDataLen)
    return -3;
  readLen = client.readBytes(response, respDataLen);
  if (readLen < respDataLen)
    return -4;
  for (int i = 0, j = 0; i < len; i++, j += 2) {
    regs[i] = response[j] * 256 + response[j + 1];
  }
  return 0;
}

int modbusWriteSingle(Client& client, unsigned int address, int val) {

  const byte CODE_IX = 7;
  const byte ERR_CODE_IX = 8;
  const byte RESPONSE_LENGTH = 9;

  client.setTimeout(2000);

  byte req[] = { 0, 1, 0, 0, 0, 6, 1, FNC_WRITE_SINGLE, // header
        (byte) (address / 256), (byte) (address % 256),
        (byte) (val / 256), (byte) (val % 256)};

  client.write(req, sizeof(req));

  byte response[RESPONSE_LENGTH];
  int readLen = client.readBytes(response, RESPONSE_LENGTH);
  if (readLen < RESPONSE_LENGTH)
    return MODBUS_NO_RESPONSE;
  switch (response[CODE_IX]) {
    case FNC_WRITE_SINGLE:
      break;
    case (FNC_ERR_FLAG | FNC_WRITE_SINGLE):
      return response[ERR_CODE_IX]; // 0x01, 0x02, 0x03, 0x04 or 0x11
    default:
      return -2;
  }
  while (client.read() != -1);
  return 0;
}

