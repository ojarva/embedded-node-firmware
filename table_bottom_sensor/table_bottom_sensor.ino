#include <JeeLib.h>
#include "settings.h"

Port motion_sensor_port (MOTION_SENSOR_BUS);
PortI2C myBus (GRAVITY_SENSOR_BUS);
GravityPlug sensor (myBus);
MilliTimer measureTimer;


long last_sent_at = millis();
int prevaxes[3] = {0, 0, 0};
int triggered_by[3];
int diffs[3] = {0, 0, 0};
int diffi = 0;
bool pir_triggered = false;

void setup () {
    Serial.begin(57600);
    Serial.print("\n[");
    Serial.print(DEVICE_DESCR);
    Serial.println("]");

    if (sensor.isPresent()) {
      Serial.print("sensor version ");
      sensor.send();
      sensor.write(0x01);
      sensor.receive();
      Serial.println(sensor.read(1), HEX);
      sensor.stop();
    }

    rf12_initialize(RF12_DEVICE_ID, RF12_868MHZ, RF12_DEVICE_GROUP);
    sensor.begin();
    sensor.sensitivity(2);
    memcpy(prevaxes, sensor.getAxes(), sizeof payload.axes);
}

int get_diff(int index) {
  return abs(payload.axes[index] - prevaxes[index]);
}

void loop () {
    if (measureTimer.poll(25)) {
      int pir_data = motion_sensor_port.anaRead();
      long now = millis();
      if (pir_data > 990) {
        if (!pir_triggered && now - last_sent_at > 1000) {
          payload.triggered_by = 1;
          rf12_sendNow(0, &payload, sizeof payload);
          last_sent_at = now;
          Serial.println("PIR triggered");
        }
        pir_triggered = true;
      } else {
        pir_triggered = false;
      }

        memcpy(payload.axes, sensor.getAxes(), sizeof payload.axes);

        triggered_by[0] = get_diff(0);
        triggered_by[1] = get_diff(1);
        triggered_by[2] = get_diff(2);
        int movement_sum = triggered_by[0] + triggered_by[1] + triggered_by[2];
        diffs[diffi] = movement_sum;
        diffi = diffi + 1;
        if (diffi > 2) {
          diffi = 0;
        }
        if (diffs[0] + diffs[1] + diffs[2] > 11) {
          if (now - last_sent_at > 1000) {
            payload.triggered_by = 2;
           rf12_sendNow(0, &payload, sizeof payload);
           last_sent_at = now;
          }
        }
        memcpy(prevaxes, payload.axes, sizeof payload.axes);
    }
}
