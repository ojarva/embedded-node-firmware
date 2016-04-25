#include <Wire.h>
#include <OneWire.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DallasTemperature.h>

const char* ssid     = "ssid";
const char* password = "wpa-key";

IPAddress stats_ip(192, 168, 10, 112);
int statsd_port = 8225;
WiFiUDP udpclient;


OneWire indoorsOneWire(D1);
DallasTemperature indoorsTemperature(&indoorsOneWire);
OneWire outdoorsOneWire(D2);
DallasTemperature outdoorsTemperature(&outdoorsOneWire);

char * buffer;

long lastDataSentAt = 0;

byte smallWindow1;
byte smallWindow2;
byte largeWindow;
byte innerDoor;
byte outerDoor;

byte csmallWindow1;
byte csmallWindow2;
byte clargeWindow;
byte cinnerDoor;
byte couterDoor;


void readCurrentSwitch() {
  csmallWindow1 = digitalRead(D3);
  clargeWindow = digitalRead(D4);
  csmallWindow2 = digitalRead(D6);
  cinnerDoor = digitalRead(D5);
  couterDoor = digitalRead(D7);
}

void setup() {
  Serial.begin(57600);
  delay(10);

  // We start by connecting to a WiFi network
  buffer = new char[128];
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  outdoorsTemperature.begin();
  indoorsTemperature.begin();
  readCurrentSwitch();
  smallWindow1 = 1 - csmallWindow1;
  smallWindow2 = 1 - csmallWindow2;
  innerDoor = 1 - cinnerDoor;
  outerDoor = 1 - couterDoor;
  largeWindow = 1 - clargeWindow;
  sendPacket("door.boot", 1);
}

void sendPacket(const char *metric, int value) {
  udpclient.beginPacket(stats_ip, statsd_port);
  memset(buffer, 0, 128);
  String stringValue(value);
  strcat(buffer, metric);
  strcat(buffer, ":");
  strcat(buffer, stringValue.c_str());
  strcat(buffer, "\n");
  Serial.print("Sending ");
  Serial.println(buffer);
  udpclient.write((uint8_t *)buffer, strlen(buffer));
  udpclient.endPacket();
}

void sendData() {
  lastDataSentAt = millis();
  Serial.print("Requesting temperatures...");
  indoorsTemperature.setResolution(12);
  indoorsTemperature.requestTemperatures();
  outdoorsTemperature.setResolution(12);
  outdoorsTemperature.requestTemperatures();
  Serial.println("done");

  sendPacket("window.ceiling-t1", indoorsTemperature.getTempCByIndex(0) * 100);
  sendPacket("window.ceiling-t2", indoorsTemperature.getTempCByIndex(1) * 100);
  sendPacket("window.ceiling-t3", indoorsTemperature.getTempCByIndex(2) * 100);
  sendPacket("window.outdoors-t1", outdoorsTemperature.getTempCByIndex(0) * 100);
}


void loop() {
  if (millis() - lastDataSentAt > 20000) {
    sendData();
  }
  readCurrentSwitch();
  if (smallWindow1 != csmallWindow1) {
    sendPacket("window.small-window-1-open-sw", csmallWindow1);
    smallWindow1 = csmallWindow1;
  }
  if (smallWindow2 != csmallWindow2) {
    sendPacket("window.small-window-2-open-sw", csmallWindow2);
    smallWindow2 = csmallWindow2;
  }
  if (largeWindow != clargeWindow) {
    sendPacket("window.large-window-open-sw", clargeWindow);
    largeWindow = clargeWindow;
  }
  if (innerDoor != cinnerDoor) {
    sendPacket("window.inner-door-open-sw", cinnerDoor);
    innerDoor = cinnerDoor;
  }
  if (outerDoor != couterDoor) {
    sendPacket("window.outer-door-open-sw", couterDoor);
    outerDoor = couterDoor;
  }
}
