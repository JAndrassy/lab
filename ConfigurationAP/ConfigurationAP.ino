/*
  Configuration AP example
  The example shows a very simple Configuration Access Point.

  created in August 2019
  by Juraj Andrassy https://github.com/jandrassy

*/
#include <ESP8266WiFi.h>

void setup() {

  Serial.begin(115200);
  delay(500);

//  WiFi.disconnect(); // forget the persistent connection to test the Configuration AP

  // waiting for connection to remembered  Wifi network
  Serial.println("Waiting for connection to WiFi");
  WiFi.waitForConnectResult();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println("Could not connect to WiFi. Starting configuration AP...");
    configAP();
  } else {
    Serial.println("WiFi connected");
  }
}

void loop() {
}

void configAP() {

  WiFiServer configWebServer(80);

  WiFi.mode(WIFI_AP_STA); // starts the default AP (factory default or setup as persistent)

  Serial.print("Connect your computer to the WiFi network ");
  Serial.print(WiFi.softAPSSID());
  Serial.println();
  IPAddress ip = WiFi.softAPIP();
  Serial.print("and enter http://");
  Serial.print(ip);
  Serial.println(" in a Web browser");

  configWebServer.begin();

  while (true) {

    WiFiClient client = configWebServer.available();
    if (client) {
      char line[64];
      int l = client.readBytesUntil('\n', line, sizeof(line));
      line[l] = 0;
      client.find((char*) "\r\n\r\n");
      if (strncmp_P(line, PSTR("POST"), strlen("POST")) == 0) {
        l = client.readBytes(line, sizeof(line));
        line[l] = 0;

        // parse the parameters sent by the html form
        const char* delims = "=&";
        strtok(line, delims);
        const char* ssid = strtok(NULL, delims);
        strtok(NULL, delims);
        const char* pass = strtok(NULL, delims);

        // send a response before attemting to connect to the WiFi network
        // because it will reset the SoftAP and disconnect the client station
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Connection: close"));
        client.println(F("Refresh: 10")); // send a request after 10 seconds
        client.println();
        client.println(F("<html><body><h3>Configuration AP</h3><br>connecting...</body></html>"));
        client.stop();

        Serial.println();
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        WiFi.begin(ssid, pass);
        WiFi.waitForConnectResult();

        // configuration continues with the next request

      } else {

        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Connection: close"));
        client.println();
        client.println(F("<html><body><h3>Configuration AP</h3><br>"));

        int status = WiFi.status();
        if (status == WL_CONNECTED) {
          client.println(F("Connection successful. Ending AP."));
        } else {
          client.println(F("<form action='/' method='POST'>WiFi connection failed. Enter valid parameters, please.<br><br>"));
          client.println(F("SSID:<br><input type='text' name='i'><br>"));
          client.println(F("Password:<br><input type='password' name='p'><br><br>"));
          client.println(F("<input type='submit' value='Submit'></form>"));
        }
        client.println(F("</body></html>"));
        client.stop();

        if (status == WL_CONNECTED) {
          delay(1000); // to let the SDK finish the communication
          Serial.println("Connection successful. Ending AP.");
          configWebServer.stop();
          WiFi.mode(WIFI_STA);
        }
      }
    }
  }
}
