#include "settings.h"
#include "helpers.h"
#include <time.h>
#include <WiFi.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void getTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();   
}