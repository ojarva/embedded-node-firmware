#include <OneWire.h>
#include <DallasTemperature.h>
#include <JeeLib.h>
#include <EmonLib.h>

#define measurePin 14

OneWire ds(6);
DallasTemperature sensors(&ds);
MilliTimer measureTimer;
MilliTimer measureTimer2;
EnergyMonitor emon1;

float powerMeasureSum = 0;
int powerMeasureCount = 0;
byte prevDoorOpen = 0;

struct {
  float temperature1;
  float temperature2;
  float power_consumption;
  byte door;
  int water;
} payload;

void setup() {
  Serial.begin(57600);
  Serial.println("\n[freezer]");
  rf12_initialize(2, RF12_868MHZ, 45);
  sensors.begin();
  pinMode(7, INPUT);
  pinMode(A3, INPUT);
  pinMode(measurePin, INPUT);
  emon1.current(measurePin, 20.2);
  delay(100);
}

void sendNow() {
  sensors.requestTemperatures();
  payload.temperature1 = sensors.getTempCByIndex(0);
  payload.temperature2 = sensors.getTempCByIndex(1);
  payload.water = analogRead(A3);
  if (powerMeasureCount > 0) {
    payload.power_consumption = powerMeasureSum / powerMeasureCount;
  } else {
    payload.power_consumption = emon1.calcIrms(1480);
  }
  powerMeasureCount = 0;
  powerMeasureSum = 0;
  printPayload();
  rf12_sendNow(0, &payload, sizeof payload);
}

void printPayload() {
  Serial.print("Temperature 1: ");
  Serial.println(payload.temperature1);
  Serial.print("Temperature 2: ");
  Serial.println(payload.temperature2);
  Serial.print("Water: ");
  Serial.println(payload.water);
  Serial.print("Door: ");
  Serial.println(payload.door);
  Serial.print("Energy: ");
  Serial.println(payload.power_consumption);
}

void loop() {
  if (measureTimer2.poll(100)) {
     powerMeasureSum += emon1.calcIrms(1480);
     powerMeasureCount++;
     payload.door = digitalRead(7);
     if (payload.door != prevDoorOpen) {
      Serial.println("Door changed - sending immediately");
        sendNow();
     }
     prevDoorOpen = payload.door;
  }
  if (measureTimer.poll(10000)) {
        sendNow();

  }
}
