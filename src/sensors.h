#pragma once

#include <SparkFun_BNO080_Arduino_Library.h>
#include <SparkFun_I2C_Mux_Arduino_Library.h>

const char fingernames[11][13] = {
  "wrist",
  "pinkieUpper",
  "pinkieLower",
  "ringUpper",
  "ringLower",
  "middleUpper",
  "middleLower",
  "indexUpper",
  "indexLower",
  "thumbUpper",
  "thumbLower",
};

const char legnames[3][13] = {
  "thigh",
  "calf",
  "foot",
};

const char backnames[3][13] = {
  "head",
  "upper",
  "lower",
};

const char armnames[3][13] = {
  "forearm",
  "bicep",
};

const char defaultnames[3][13] = {
  "sensor",
};

const char LEFTHAND[] = "lefthand";
const char RIGHTHAND[] = "righthand";
const char LEFTARM[] = "leftarm";
const char RIGHTARM[] = "rightarm";
const char LEFTLEG[] = "leftleg";
const char RIGHTLEG[] = "rightleg";
const char TORSO[] = "torso";
const char DEFAULT_SENSOR[] = "default";
const char *getSensors();

