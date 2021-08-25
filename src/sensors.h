#pragma once

#include "chuuni.h"

#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <SparkFun_BNO080_Arduino_Library.h>

const char *fingernames[] = {
    "pinkieUpper", "pinkieLower", "ringUpper", "ringLower",
    "middleUpper", "middleLower", "indexUpper", "indexLower",
    "thumbUpper",  "thumbLower", "wrist", "forearm"
};

const char *legnames[] = {
    "thigh",
    "calf",
    "foot",
};

const char *backnames[] = {
    "head",
    "upper",
    "lower",
};

const char *armnames[] = {
    "forearm",
    "bicep",
};

const char *defaultnames[] = {
    "sensor",
};

const char **getSensorsNames() {
  if (PART == "lefthand") {
    return fingernames;
  } else if (PART == "leftleg" || PART == "rightleg") {
    return legnames;
  } else if (PART == "leftarm" || PART == "rightarm") {
    return armnames;
  } else if (PART == "torso") {
    return backnames;
  } else {
    Serial.println("PART not defined!");
    return defaultnames;
  }
}
