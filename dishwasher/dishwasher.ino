#include <OneWire.h>
#include <DallasTemperature.h>

#include <EmonLib.h>

#include <JeeLib.h>
MilliTimer measureTimer;
EnergyMonitor emon1;

#define measurePin A0

OneWire oneWire_in(7);
DallasTemperature sensor_down(&oneWire_in);

OneWire oneWire_shower(6);
DallasTemperature sensor_shower(&oneWire_shower);

byte lastDoorOpen;

long lastSent = millis();
float prevValue = 0;
struct {
  float value;
  float temperature1;
  float temperature2;
  float temperature3;
  float temperature4;
  byte door;
} payload;

void setup() {
  Serial.begin(57600);
  Serial.println("\n[dishwasher_power]");
  rf12_initialize(4, RF12_868MHZ, 45);
  pinMode(A2, INPUT);
  analogRead(A2);
  analogRead(A2);
  pinMode(measurePin, INPUT);
  analogRead(measurePin);
  analogRead(measurePin);
  emon1.current(measurePin, 21.9);
  emon1.calcIrms(1480);
  emon1.calcIrms(1480);
  sensor_down.begin();
  sensor_shower.begin();
  delay(250);
}

void sendNow() {
  sensor_down.requestTemperatures();
  sensor_shower.requestTemperatures();
  payload.temperature1 = sensor_down.getTempCByIndex(0);
  payload.temperature2 = sensor_down.getTempCByIndex(1);
  payload.temperature3 = sensor_down.getTempCByIndex(2);
  payload.temperature4 = sensor_shower.getTempCByIndex(0);
  rf12_sendNow(0, &payload, sizeof payload);
  lastSent = millis();  
}

void loop() {
  if (measureTimer.poll(1000)) {
      float value = emon1.calcIrms(1480);
      byte doorOpen = analogRead(A2);
      if (doorOpen > 128) {
        doorOpen = 1;
      } else {
        doorOpen = 0;
      }
      payload.door = doorOpen;
      payload.value = value;
      Serial.print("Power consumption ");
      Serial.println(value);
      if (doorOpen != lastDoorOpen) {
        Serial.println("Door status changed. Sending.");
        sendNow();  
      } else if (abs(prevValue - value) > 0.3) {
        // Change in power consumption was large enough -> send immediately.
        Serial.println("Sending - power consumption changed significantly");
        sendNow();
      } else {
        if (millis() - lastSent > 10000) {
          Serial.println("Sending - timeout exceeded");
          // Last transmission is too old - send an update        
          sendNow();
        }
      }
      lastDoorOpen = doorOpen;
      prevValue = value;
  }
}


