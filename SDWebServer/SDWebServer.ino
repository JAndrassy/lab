#include <Ethernet.h>
#include <SD.h>
#include <StreamLib.h> // install in Library Manager. Used to generate HTML of directory listing

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

const int SDCARD_CS = 4;
const int ETHERNET_CS = 10;
const int DOWNLOAD_BUFFER_SIZE = 512;

EthernetServer server(80);

void setup() {

  Serial.begin(115200);
  while (!Serial);

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  Ethernet.init(ETHERNET_CS);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    while (true) {
       delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  if (!SD.begin(SDCARD_CS)) {
    Serial.println(F("SD card initialization failed!"));
    // don't continue
    while (true);
  }

  server.begin();

  IPAddress ip = Ethernet.localIP();
  Serial.println();
  Serial.println(F("Connected to network."));
  Serial.print(F("To access the server, enter \"http://"));
  Serial.print(ip);
  Serial.println(F("/\" in web browser."));

}

void loop() {

  EthernetClient client = server.available();

  if (client && client.connected()) {
    if (client.find(' ')) { // GET /fn HTTP/1.1
      char fn[32];
      int l = client.readBytesUntil(' ', fn, sizeof(fn) - 1); // read the filename from URL
      fn[l] = 0;
      client.find((char*) "\r\n\r\n");
      File file = SD.open(fn);
      if (!file) { // file was not found
        client.println(F("HTTP/1.1 404 Not Found"));
        client.println(F("Connection: close"));
        client.print(F("Content-Length: "));
        client.println(strlen(" not found") + strlen(fn));
        client.println();
        client.print(fn);
        client.print(F(" not found"));
      } else if (file.isDirectory()) {
        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Connection: close"));
        client.println(F("Content-Type: text/html"));
        client.println(F("Transfer-Encoding: chunked"));
        client.println();
        char buff[64]; // buffer for chunks
        ChunkedPrint chunked(client, buff, sizeof(buff));
        chunked.begin();
        chunked.printf(F("<!DOCTYPE html>\r\n<html>\r\n<body>\r\n<h3>Folder '%s'</h3>\r\n"), fn);
        while (true) {
          File entry = file.openNextFile();
          if (!entry)
            break;
          if (strcmp(fn, "/") == 0) {
            chunked.printf(F("<a href='%s'>"), entry.name());
          } else {
            chunked.printf(F("<a href='%s/%s'>"), fn, entry.name());
          }
          chunked.print(entry.name());
          if (entry.isDirectory()) {
            chunked.println(F("/</a><br>"));
          } else {
            chunked.printf(F("</a> (%ld b)<br>\r\n"), entry.size());
          }
          entry.close();
        }
        chunked.println(F("</body>\r\n</html>"));
        chunked.end();
      } else {
        char buff[DOWNLOAD_BUFFER_SIZE];
        BufferedPrint bp(client, buff, sizeof(buff));
        bp.println(F("HTTP/1.1 200 OK"));
        bp.println(F("Connection: close"));
        bp.print(F("Content-Length: "));
        bp.println(file.size());
        bp.print(F("Content-Type: "));
        const char* ext = strchr(file.name(), '.');
        bp.println(getContentType(ext));
        bp.println();
        while (file.available()) {
          bp.write(file.read()); // send the file as body of the response
        }
        bp.flush();
        file.close();
      }
    }
    client.stop();
  }
}

const char* getContentType(const char* ext){
  if (!strcmp(ext, ".HTM"))
    return "text/html";
  if (!strcmp(ext, ".CSS"))
    return "text/css";
  if (!strcmp(ext, ".JS"))
    return "application/javascript";
  if (!strcmp(ext, ".PNG"))
    return "image/png";
  if (!strcmp(ext, ".GIF"))
    return "image/gif";
  if (!strcmp(ext, ".JPG"))
    return "image/jpeg";
  if (!strcmp(ext, ".XML"))
    return "text/xml";
  return "text/plain";
}
