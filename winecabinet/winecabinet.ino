#include <RF12.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <SI7021.h>

#include <EmonLib.h>

#include <JeeLib.h>

#define ONEWIRE_INSIDE_PIN 7 // P4 D
#define DOOR_SENSOR_PIN A3

OneWire oneWire_in(ONEWIRE_INSIDE_PIN);
DallasTemperature sensors_inside(&oneWire_in);
long lastSentDoorAt = 0;
MilliTimer measureTimer;

SI7021 si7021_sensor;


struct {
  float consumption;
  float temperature1; // cooling element
  float temperature2; // upper sensor
  float temperature3; // control panel
  float temperature4; // floor
  float temperature5; // lower sensor
  float rh_temp;
  float rh_humid;
  byte doorIsOpen;
} payload;

void setup() {
  Serial.begin(57600);
  Serial.println("\n[winecabinet]");
  rf12_initialize(10, RF12_868MHZ, 45); // TODO: id
  payload.doorIsOpen = readDoor();
  sensors_inside.begin();
  si7021_sensor.begin();
  delay(250);
}

byte readDoor() {
  return 1 - digitalRead(DOOR_SENSOR_PIN);
}

void sendNow() {
  sensors_inside.requestTemperatures();
  payload.doorIsOpen = readDoor();
  payload.temperature1 = sensors_inside.getTempCByIndex(0);
  payload.temperature2 = sensors_inside.getTempCByIndex(1);
  payload.temperature3 = sensors_inside.getTempCByIndex(2);
  payload.temperature4 = sensors_inside.getTempCByIndex(3);
  payload.temperature5 = sensors_inside.getTempCByIndex(4);
  si7021_env data = si7021_sensor.getHumidityAndTemperature();
  payload.rh_temp = float(data.celsiusHundredths) / 100;
  payload.rh_humid = float(data.humidityBasisPoints) / 100;
  printPayload();
  rf12_sendNow(0, &payload, sizeof payload);
}

void printPayload() {
  Serial.print("Temperature 1: ");
  Serial.println(payload.temperature1);
  Serial.print("Temperature 2: ");
  Serial.println(payload.temperature2);
  Serial.print("Temperature 3: ");
  Serial.println(payload.temperature3);
  Serial.print("Temperature 4: ");
  Serial.println(payload.temperature4);
  Serial.print("Temperature 5: ");
  Serial.println(payload.temperature5);
  Serial.print("Humidity (%): ");
  Serial.println(payload.rh_humid);
  Serial.print("Humidity sensor temp (C): ");
  Serial.println(payload.rh_temp);
  Serial.print("Door is open: ");
  Serial.println(payload.doorIsOpen);
  Serial.print("Power consumption (A): ");
  Serial.println(payload.consumption);
}


void loop() {
  byte currentDoor = readDoor();
  long now = millis();
  if (currentDoor != payload.doorIsOpen && now - lastSentDoorAt > 1000) {
    sendNow();
    lastSentDoorAt = now;
  }
  if (measureTimer.poll(10000)) {
    sendNow();
  }
}
