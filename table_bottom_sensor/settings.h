#define GRAVITY_SENSOR_BUS 4
#define MOTION_SENSOR_BUS 2
#define RF12_DEVICE_ID 7
#define RF12_DEVICE_GROUP 45
#define DEVICE_DESCR "table_bottom_sensor"

struct {
  int axes[3];
  byte triggered_by;
} payload;

