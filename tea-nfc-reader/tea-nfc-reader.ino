#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
TwoWire wiretmp;

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

const char* ssid     = "ssid";
const char* password = "pwa key";

char * buffer;
IPAddress stats_ip(192, 168, 10, 112);
int statsd_port = 8225;
WiFiUDP udpclient;

void setup(void) {
  Serial.begin(115200);

  // Ugly hack to reset PN532.
  digitalWrite(D3, HIGH);
  delay(20);
  digitalWrite(D3, LOW);
  delay(20);
  digitalWrite(D4, HIGH);
  delay(20);
  digitalWrite(D4, LOW);
  delay(20);

  Wire.setClockStretchLimit(2000);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("PN53X is not available.");
    while (1); // will crash ESP8266 after a while, and reboot automatically.
  }

  nfc.setPassiveActivationRetries(0x19);
  nfc.SAMConfig();

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


  Serial.println("Waiting for an ISO14443A Card ...");
}


String convertTmp;

void sendPacket(uint8_t *uid, uint8_t uidLength) {
  udpclient.beginPacket(stats_ip, statsd_port);
  memset(buffer, 0, 128);
  strcat(buffer, "tea.nfc-id");
  strcat(buffer, ":");
  for (uint8_t i = 0; i < uidLength; i++) {
    convertTmp = String(uid[i], HEX);
    if (convertTmp.length() == 1) {
      convertTmp = "0" + convertTmp;
    }
    strcat(buffer, convertTmp.c_str());
  }
  strcat(buffer, "\n");
  udpclient.write((uint8_t *)buffer, strlen(buffer));
  udpclient.endPacket();
}


uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;

void loop(void) {

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.print("ID: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    sendPacket(uid, uidLength);
  }
  delay(250);
}
