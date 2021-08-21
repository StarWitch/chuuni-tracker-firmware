#include "settings.h"
#include "helpers.h"
#include "sensors.h"

#include <OSCBundle.h>
#include <OSCMessage.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

QWIICMUX myMux;

WiFiUDP Udp;

const char names[6][11] = { "leftPinkie", "leftRing",  "leftMiddle", "leftIndex", "leftThumb", "leftWrist" };

void setup() {
  pinMode(IO_ENABLE, OUTPUT);
  digitalWrite(IO_ENABLE, HIGH);
  #ifdef WIFI_IMU_DEBUG
  Serial.begin(115200);
  #endif

  #ifndef WIFI_DISABLE
  // connect to WiFi
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.mode(WIFI_OFF);
  WiFi.disconnect();
  delay(1000);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");

  Serial.println("Try to connect to NTP-server and get time  ");
  getTime();
  #endif

  Serial.println("Begin wire!");
  Wire.begin(SDA_EXT, SCL_EXT);
  Serial.println("Began wire!");

  Wire.setClock(400000); // Increase I2C data rate to 400kHz

  //  //================================

  IMUSensor = new BNO080 *[NUMBER_OF_SENSORS];

  for (int x = 0; x < NUMBER_OF_SENSORS; x++) {
    IMUSensor[x] = new BNO080();
  }
  Serial.println("Allocated sensors!");

  if (myMux.begin() == false) {
    Serial.println("Mux not detected. Freezing...");
    while (1)
      ;
  }
  delay(500);
  // Initialize all the sensors
  bool initSuccess = true;
  bool enabled;
  for (int x = 0; x < NUMBER_OF_SENSORS; x++) {
    myMux.setPort(x);
    delay(300);
    enabled = IMUSensor[x]->begin(0x4B);
    delay(100);
    /*
    if (enabled == false) // Begin returns 0 on a good init
    {
      enabled = IMUSensor[x]->begin(0x4B);
      enabled = IMUSensor[x]->begin(0x4B);
      enabled = IMUSensor[x]->begin(0x4B);
*/
      if (enabled == false) {
        Serial.print("Sensor ");
        Serial.print(x);
        Serial.println(" did not begin! Check wiring");
        initSuccess = false;
      } else {
        // Configure each sensor
        IMUSensor[x]->enableRotationVector(1); // Send data update every 10ms
        Serial.print("Sensor ");
        Serial.print(x);
        Serial.println(" configured");
      }
  }

  if (initSuccess == false) {
    Serial.print("Freezing...");
    while (1)
      ;
  }

  Serial.println("Mux Shield online");
  Serial.println(F("Rotation vector enabled"));
  Udp.begin(8888);
}


void loop() {
  OSCBundle bundle;
  // Look for reports from the IMU

  for (int x = 0; x < NUMBER_OF_SENSORS; x++) {
    myMux.setPort(x);
    if (IMUSensor[x]->dataAvailable() == true) {
      //Serial.print(names[x]);
      //Serial.print(": ");
      //Serial.println(x);
      float quatI = IMUSensor[x]->getQuatI();
      float quatJ = IMUSensor[x]->getQuatJ();
      float quatK = IMUSensor[x]->getQuatK();
      float quatReal = IMUSensor[x]->getQuatReal();
      bundle.add("/hand").add(names[x]).add(quatI).add(quatJ).add(quatK).add(quatReal);
    } else {
      Serial.print("data not available for: ");
      Serial.print(names[x]);
      Serial.print(" ");
      Serial.println(x);
    }
  }
  
  Udp.beginPacket(OSC_HOST, OSC_HOST_PORT);
  bundle.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket();  // mark the end of the OSC Packet
  bundle.empty();   // empty the bundle to free room for a new one
}
