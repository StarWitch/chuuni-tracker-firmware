#pragma once

// NOTE: edit "user_config.h" instead of editing things here!!
#ifdef CHUUNI_USER_CONFIG
#include "user_config.h"
#endif

// sensible defaults. edit "user_config.h" to override!

// unsure if these two do anything on the Arduino framework
#ifdef CORE_DEBUG_LEVEL
#undef CORE_DEBUG_LEVEL
#endif

#ifdef LOG_LOCAL_LEVEL
#undef LOG_LOCAL_LEVEL
#endif

#define CORE_DEBUG_LEVEL 3
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#ifndef SERIAL_DEBUG
#define SERIAL_DEBUG false
#endif

#ifndef I2C_DEBUG
#define I2C_DEBUG false
#endif

// WiFi section

#if !defined(WIFI_SSID) || !defined(WIFI_PASS)
#define WIFI_SSID "DefaultSSID"
#define WIFI_PASS "DefaultPass"
#endif

#ifndef WIFI_DISABLE
#define WIFI_DISABLE false
#endif

// IP of the host (i.e: computer running Unity)
#ifndef OSC_HOST
#define OSC_HOST "192.168.2.1"
#endif

// what port the host is expecting data on (i.e: Unity)
#ifndef OSC_HOST_PORT
#define OSC_HOST_PORT 9000
#endif

// what port the ESP32S2 sends data on
#ifndef OSC_CLIENT_PORT
#define OSC_CLIENT_PORT 8888
#endif

// Pin section

// external I2C
#if !defined(SCL_EXT) || !defined(SDA_EXT)
#define SCL_EXT 34
#define SDA_EXT 33
#endif

// internal I2C
#if !defined(SCL_INT) || !defined(SDA_INT)
#define SCL_INT 36
#define SDA_INT 35
#endif

// for enabling the second 3.3V regulator (i.e: IMUs, I2C)
#ifndef IO_ENABLE_PIN
#define IO_ENABLE_PIN 37
#endif

// internal/onboard BNO085 reset pin
#ifndef BNO_INT_RESET_PIN
#define BNO_INT_RESET_PIN 16
#endif

// neopixel status pin
#ifndef NEOPIXEL_PIN
#define NEOPIXEL_PIN 45
#endif

// voltage divider pin (measuring battery level through ADC)
#ifndef VDIV_PIN
#define VDIV_PIN 5
#endif

// debug start pin (for toggling debug mode, third button)
#ifndef OPT_BUTTON_PIN
#define OPT_BUTTON_PIN 6
#endif

#ifndef BOOT_BUTTON_PIN
#define BOOT_BUTTON_PIN 0
#endif

// misc constants

#define VOLTAGE_READ_CONSTANT 1536.0

// IMU section

// in hertz, has to be at least 1
#define IMU_UPDATE_RATE 1

// for external IMUs connected to a TCA9458 multiplexer
#ifndef MUX_DISABLE
#define MUX_DISABLE false
#endif

#ifndef INTERNAL_IMU_ENABLE
#define INTERNAL_IMU_ENABLE true // we generally want the internal IMU to be enabled
#endif

#ifndef EXTERNAL_IMU_ENABLE
#define EXTERNAL_IMU_ENABLE false // needs to be explicitly defined
#endif

#ifdef LEFTHAND
#define PART "/lefthand"
#define NUMBER_OF_SENSORS 12
#endif
#ifdef RIGHTHAND
#define PART "/righthand"
#define NUMBER_OF_SENSORS 12
#endif
#ifdef LEFTLEG
#define PART "/leftleg"
#define NUMBER_OF_SENSORS 3
#endif
#ifdef RIGHTLEG
#define PART "/rightleg"
#define NUMBER_OF_SENSORS 3
#endif
#ifdef LEFTARM
#define PART "/leftarm"
#define NUMBER_OF_SENSORS 2
#endif
#ifdef RIGHTARM
#define PART "/rightarm"
#define NUMBER_OF_SENSORS 2
#endif
#ifdef TORSO
#define PART "/torso"
#define NUMBER_OF_SENSORS 3
#endif

// set up default sensor
#ifndef PART
#define PART "/default"
#endif

#ifndef NUMBER_OF_SENSORS
#define NUMBER_OF_SENSORS 1
#endif

// defining global objects for use

#include <WiFi.h>
#include <WiFiudp.h>
#include <Wire.h>

#include <OSCBundle.h>
#include <OSCMessage.h>

#include <SparkFun_I2C_Mux_Arduino_Library.h>
#include <SparkFun_BNO080_Arduino_Library.h>
#include <Adafruit_NeoPixel.h>

extern QWIICMUX i2c_muxer;
extern WiFiUDP udp;
extern BNO080 **imu_sensors;

extern const char **sensornames;

extern Adafruit_NeoPixel pixel;

extern bool debug_mode;
