#ifndef wifi_imu_helpers_h
#define wifi_imu_helpers_h

#ifdef WIFI_IMU_DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(x...) Serial.println(x)
#endif

void getTime();

#endif // wifi_imu_helpers_h