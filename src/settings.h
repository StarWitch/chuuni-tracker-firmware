#pragma once

/* 

pinouts for ChuuniTracker

IO5 = Voltage Divider
IO6 = Button (OPT)
IO8 = TCA9548 RST pin
IO9 = I2C_EN (v1.5.1)
IO10 = I2C_READY (v1.5.1)
IO16 = BNO085 Reset (Onboard)
IO21 = BNO085 Interrupt (Onboard)
IO33 = SDA_EXT
IO34 = SCL_EXT
IO35 = SDA_INT
IO36 = SCL_INT
IO37 = VCCIO Enable (Required HIGH for SDA_EXT/SCL_EXT)
IO38 = 1-bit mem for reset
IO45 = Status NeoPixel

*/

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

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
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
#define BNO_INT_RST 7
#endif

#ifndef MUX_DISABLE
#define MUX_DISABLE 0
#endif