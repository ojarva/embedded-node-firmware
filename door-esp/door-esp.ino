#include <Wire.h>
#include <OneWire.h>

#include <ESP8266WiFi.h>
#include <DallasTemperature.h>


#define INNER_DOOR_PIN D1
#define OUTER_DOOR_PIN D2

const char* ssid     = "ssid-lan";
const char* password = "wifi key";

const char* host = "192.168.10.112";

OneWire oneWire(D5);
DallasTemperature sensors(&oneWire);

long lastDataSentAt = 0;

byte door_outer;
byte door_inner;

void setup() {
  Serial.begin(57600);
  delay(10);
  
  pinMode(INNER_DOOR_PIN, INPUT_PULLUP);
  pinMode(OUTER_DOOR_PIN, INPUT_PULLUP);

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

  sensors.begin();

  door_outer = digitalRead(OUTER_DOOR_PIN);
  door_inner = digitalRead(INNER_DOOR_PIN);
}


void sendData() {
  lastDataSentAt = millis();

  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("done");

  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  door_outer = digitalRead(OUTER_DOOR_PIN);
  door_inner = digitalRead(INNER_DOOR_PIN);
  
  // We now create a URI for the request
  String request = "GET /nodes/door?door_outer=";
  request += String(door_outer);
  request += String("&door_inner=") + String(door_inner);
  request += "&t1=";
  request += sensors.getTempCByIndex(0);
  request += "&end";
  request += " HTTP/1.1\r\n";
  request += "Host: " + String(host) + "\r\n";
  request += "Connection: close\r\n\r\n";

  Serial.print("Request: ");
  Serial.println(request);
  
  // This will send the request to the server
  client.print(request);
  delay(5000);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void loop() {
  byte door_outer_current = digitalRead(OUTER_DOOR_PIN);
  byte door_inner_current = digitalRead(INNER_DOOR_PIN);
  if (door_outer_current != door_outer) {
    Serial.println("Outer door triggered");
    sendData();
  } else if (door_inner_current != door_inner) {
    Serial.println("Inner door triggered");
    sendData();
  }

  long timeElapsed = millis() - lastDataSentAt;
  if (timeElapsed < 20000) {
    delay(1000);
    return;
  }
  Serial.print("Time elapsed since last transmission: ");
  Serial.print(timeElapsed);
  Serial.println("ms");
  Serial.println("Timeout triggered");
  sendData();
}

