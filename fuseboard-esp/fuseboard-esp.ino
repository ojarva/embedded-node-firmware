#include <SoftwareSerial.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "ssid-lan";
const char* password = "wifi key";

const char* host = "192.168.10.112";

SoftwareSerial swSer(D6, D5); // RX, TX

IPAddress stats_ip(192, 168, 10, 112); // IP Address of StatsD Server
int statsd_port = 8225;
WiFiUDP udpclient;

String powerData;
int powerDataLength = 0;

char * buffer;

long lastDataSentAt = 0;

void setup() {
  Serial.begin(57600);
  delay(10);
  swSer.begin(4800);
  delay(10);
  buffer = new char[128];

  // We start by connecting to a WiFi network
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
}


void sendPacket(const char *metric, const char *powerData) {
  udpclient.beginPacket(stats_ip, statsd_port);
  memset(buffer, 0, 128);
  strcat(buffer, metric);
  strcat(buffer, ":");
  strcat(buffer, powerData);
  strcat(buffer, "\n");
  udpclient.write((uint8_t *)buffer, strlen(buffer));
  udpclient.endPacket();
}


void sendData() {
  lastDataSentAt = millis();
  sendPacket("fuseboard.readings", powerData.c_str());
  powerData = "";
  powerDataLength = 0;

}

void loop() {
  while (swSer.available()) {
    char received = swSer.read();
    if (received == 'B') {
      powerData = "";
      powerDataLength = 0;
    } else if (received != '\n') {
      if (powerDataLength > 60) {
        Serial.print("Invalid power data: ");
        Serial.println(powerData);
      } else {
        Serial.print("Received (power): ");
        Serial.println(received);
        powerData += received;
        powerDataLength++;
      }
    }
    delay(20);
  }

  if (powerDataLength > 10) {
    Serial.print("Sending ");
    Serial.println(powerData);
    sendData();
  }
}

