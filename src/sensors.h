#pragma once

#include "chuuni.h"

#include "SparkFun_BNO080_Arduino_Library.h"

const char **get_sensor_names();
void i2c_scanner(TwoWire *scan_wire);
void get_sensors();
void calibration_mode();
