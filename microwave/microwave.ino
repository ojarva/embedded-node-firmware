#include <EmonLib.h>

#include <JeeLib.h>
MilliTimer measureTimer;
MilliTimer measureTimer2;
EnergyMonitor emon1;

#define measurePin A0
#define doorPin 5

#define SEND_INTERVAL 10000
long lastSent = millis() - SEND_INTERVAL;
double prevValue = 0;
bool doorOpen;
struct {
  float value;
  byte doorOpen;
} payload;

void setup() {
  Serial.begin(57600);
  Serial.println("\n[microwave]");
  rf12_initialize(6, RF12_868MHZ, 45);
  pinMode(doorPin, INPUT);
  pinMode(measurePin, INPUT);
  analogRead(measurePin);
  analogRead(measurePin);
  delay(500);
  emon1.current(measurePin, 20.2);
  emon1.calcIrms(1480);
  delay(100);
  emon1.calcIrms(1480);
  delay(100);
  prevValue = emon1.calcIrms(1480);
  delay(100);
  bool doorOpenNow = isDoorOpen();
}

void sendNow() {
    rf12_sendNow(0, &payload, sizeof payload);
    lastSent = millis();  
}

bool isDoorOpen() {
  return !digitalRead(doorPin);
}

void loop() {
  if (measureTimer2.poll(50)) {
    bool doorOpenNow = isDoorOpen();
    if (doorOpenNow != doorOpen) {
      Serial.print("Door state changed to ");
      if (doorOpenNow) {
        Serial.println("open.");
      } else {
        Serial.println("closed.");
      }
      doorOpen = doorOpenNow;
      payload.doorOpen = doorOpenNow;
      sendNow();
    }
  }
  if (measureTimer.poll(200)) {
      double value = emon1.calcIrms(1480);
      float absDiff = abs(prevValue - value);
      Serial.print("Power consumption: ");
      Serial.print(value);
      Serial.print("A, diff is ");
      Serial.println(absDiff);
      payload.value = value;
      if (value > 0.3 && millis() - lastSent > 2000) {
        Serial.println("Sending - microwave is running");
        sendNow();
      } else if (absDiff > 1) {
        Serial.println("Sending - power consumption changed significantly");
        sendNow();
      } else if (prevValue < 0.5 && absDiff > 0.05) {
        Serial.println("Sending - started up");
        sendNow();
      } else if (absDiff > 0.05 && value < 0.5) {
        Serial.println("Sending - idle power consumption changed significantly");
        sendNow();
      } else {
        if (millis() - lastSent > SEND_INTERVAL) {
          Serial.println("Sending - timeout exceeded");
          // Last transmission is too old - send an update        
          sendNow();
        }
      }
      prevValue = value;
  }
}


