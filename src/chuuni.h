#pragma once

// unsure if these two do anything on the Arduino framework
#ifdef CORE_DEBUG_LEVEL
#undef CORE_DEBUG_LEVEL
#endif

#define CORE_DEBUG_LEVEL 3
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#ifndef OSC_HOST
#define OSC_HOST "192.168.175.81"
#endif

#ifndef OSC_HOST_PORT
#define OSC_HOST_PORT 9000
#endif

#ifndef OSC_CLIENT_PORT
#define OSC_CLIENT_PORT 8888
#endif

#if !defined(SCL_EXT) || !defined(SDA_EXT)
#define SCL_EXT 34
#define SDA_EXT 33
#endif

#if !defined(SCL_INT) || !defined(SDA_INT)
#define SCL_INT 36
#define SDA_INT 35
#endif

#ifndef IO_ENABLE_PIN
#define IO_ENABLE_PIN 37
#endif

#ifndef BNO_INT_RST
#define BNO_INT_RST 16
#endif

#ifndef NUMBER_OF_SENSORS
#define NUMBER_OF_SENSORS 1
#endif

#ifndef MUX_DISABLE
#define MUX_DISABLE 0
#endif

#ifdef CHUUNI_USER_CONFIG
#include "user_config.h"
#else
#define WIFI_SSID "DefaultSSID"
#define WIFI_PASS "DefaultPass"
#endif

#ifndef I2C_DEBUG
#define I2C_DEBUG 0
#endif

#ifndef WIFI_DISABLE
#define WIFI_DISABLE 0
#endif

#ifndef WIFI_IMU_DEBUG
#define WIFI_IMU_DEBUG 0
#endif

#ifndef INT_IMU
#define INT_IMU 0
#endif

#ifndef MUX_DISABLE
#define MUX_DISABLE 0
#endif

#ifndef NUMBER_OF_SENSORS
#define NUMBER_OF_SENSORS 1
#endif

#ifndef IMU_PART_NAME
#define IMU_PART_NAME "default"
#endif
