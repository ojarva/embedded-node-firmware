
#include <JeeLib.h>
#include <EmonLib.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX


MilliTimer measureTimer;
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;
EnergyMonitor emon4;
EnergyMonitor emon5;
EnergyMonitor emon6;

struct {
  float energy1;
  float energy2;
  float energy3;
  float energy4;
  float energy5;
  float energy6;
} payload;


void setup() {
    Serial.begin(57600);
    Serial.println("\n[fuseboard-arduino]");
    pinMode(A0, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);


    mySerial.begin(4800);

    emon1.current(A0, 85.1);
    emon2.current(A1, 85.8);
    emon3.current(A2, 86.6);
    emon4.current(A3, 108.7);
    emon5.current(A4, 108.7);
    emon6.current(A5, 108.7);
    delay(100);
    readAll();
    delay(100);
    readAll();
    delay(100);
    readAll();
    delay(100);
    readAll();
    delay(100);
}

void readAll() {
  payload.energy1 = emon1.calcIrms(1480);
  payload.energy2 = emon2.calcIrms(1480);
  payload.energy3 = emon3.calcIrms(1480);
  payload.energy4 = emon4.calcIrms(1480);
  payload.energy5 = emon5.calcIrms(1480);
  payload.energy6 = emon6.calcIrms(1480);
}

void sendNow() {
  Serial.print("B-");
  Serial.print("E1=");
  Serial.print(payload.energy1);
  Serial.print(";E2=");
  Serial.print(payload.energy2);
  Serial.print(";E3=");
  Serial.print(payload.energy3);
  Serial.print(";E4=");
  Serial.print(payload.energy4);
  Serial.print(";E5=");
  Serial.print(payload.energy5);
  Serial.print(";E6=");
  Serial.print(payload.energy6);
  Serial.println();
  
  mySerial.print("B");
  mySerial.print("E1=");
  mySerial.print(payload.energy1);
  mySerial.print("|E2=");
  mySerial.print(payload.energy2);
  mySerial.print("|E3=");
  mySerial.print(payload.energy3);
  mySerial.print("|E4=");
  mySerial.print(payload.energy4);
  mySerial.print("|E5=");
  mySerial.print(payload.energy5);
  mySerial.print("|E6=");
  mySerial.print(payload.energy6);
  mySerial.println();  
}

void loop() {
  if (measureTimer.poll(10000)) {
    readAll();
    sendNow();
  }
}
