#pragma once

#include "chuuni.h"

#include "SparkFun_BNO080_Arduino_Library.h"

typedef struct ChuuniSensor {
  BNO080 *sensor;
  TwoWire *wire;
  const int muxport;
  const uint8_t address;
  const char name[20];
  int failtimes;
} chuunisensor;

extern bool mux_start;

void mux_scanner(TwoWire *scan_wire);
void i2c_scanner(TwoWire *scan_wire);
void init_mux();
void init_sensors();
void calibration_mode();
ChuuniSensor *get_sensor_list();
