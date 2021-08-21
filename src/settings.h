#pragma once

#ifndef OSC_HOST
#define OSC_HOST "192.168.2.1"
#endif

#ifndef OSC_HOST_PORT
#define OSC_HOST_PORT 9000
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

#ifndef IO_ENABLE
#define IO_ENABLE 37
#endif