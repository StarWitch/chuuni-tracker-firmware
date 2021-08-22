#include "chuuni.h"
#include "sensors.h"

const char *getSensors() {
  Serial.print("Detected part: ");
  Serial.println(IMU_PART_NAME);
  if (IMU_PART_NAME == "lefthand" || IMU_PART_NAME == "righthand") {
    return *fingernames;
  } else if (IMU_PART_NAME == "leftleg" || IMU_PART_NAME == "rightleg") {
    return *legnames;
  } else if (IMU_PART_NAME == "leftarm" || IMU_PART_NAME == "rightarm") {
    return *armnames;
  } else if (IMU_PART_NAME == "torso") {
    return *backnames;
  } else {
    Serial.println("IMU_PART_NAME not defined!");
    return *defaultnames;
  }
}
