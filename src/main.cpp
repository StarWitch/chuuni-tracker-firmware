#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include <OSCBundle.h>
#include <OSCMessage.h>

#include "chuuni.h"

#include "helpers.h"
#include "sensors.h"

QWIICMUX myMux;
WiFiUDP Udp;
BNO080 **IMUSensor;

const char **sensornames;
const int start = INT_IMU;

void setup() {
  if (WIFI_IMU_DEBUG) {
    Serial.begin(115200);
  }

  // enable IO 3.3v regulator
  pinMode(IO_ENABLE_PIN, OUTPUT);
  digitalWrite(IO_ENABLE_PIN, HIGH);
  delay(1000);

  // reset internal BNO085
  pinMode(BNO_INT_RST, OUTPUT);
  digitalWrite(BNO_INT_RST, HIGH);
  delay(50);
  digitalWrite(BNO_INT_RST, LOW);
  delay(50);

  sensornames = getSensorsNames();

  Serial.println("Begin wire!");
  Wire.begin(SDA_EXT, SCL_EXT, 400000); // sda, scl, frequency (400kHz)
  Wire1.begin(SDA_INT, SCL_INT, 400000);
  Serial.println("Began wire!");

  if (I2C_DEBUG == true) {
    i2cScanner(&Wire, &Wire1);
  }

  // allocate sensors
  IMUSensor = new BNO080 *[NUMBER_OF_SENSORS];
  for (int x = 0; x < NUMBER_OF_SENSORS; x++) {
    IMUSensor[x] = new BNO080();
  }
  Serial.println("Allocated sensors!");

  // Initialize all the sensors
  bool initSuccess = true;
  bool enabled;

  if (INT_IMU) {
    enabled = IMUSensor[0]->begin(0x4B, Wire1); // internal sensor on the WiFi board
    if (enabled == false) {
      Serial.println("Failed to start onboard IMU");
    } else {
      Serial.println("IMU 0 (Onboard) configured");
    }
  }

  if (!MUX_DISABLE) {
    // on "external" Wire interface
    if (myMux.begin(QWIIC_MUX_DEFAULT_ADDRESS, Wire) == false) {
      Serial.println("Mux not detected. Freezing...");
      while (1) {;}
    }
    delay(500); // wait for mux to truly begin

    for (int x = start; x < NUMBER_OF_SENSORS; x++) {
      Serial.print("Mux Port ");
      Serial.println(x - start);
      myMux.setPort(x - start);
      delay(300);
      enabled = IMUSensor[x]->begin(0x4B, Wire);
      delay(100);
      if (enabled == false) {
        Serial.print("Sensor ");
        Serial.print(x);
        Serial.println(" did not begin! Check wiring");
        initSuccess = false;
      } else {
        IMUSensor[x]->enableRotationVector(IMU_UPDATE_RATE);
        Serial.print("IMU ");
        Serial.print(x);
        Serial.println(" configured");
      }
    }

    if (initSuccess == false) {
      Serial.print("Failed to initialize. Freezing...");
      while (1) {;}
    }
  }

  if (!WIFI_DISABLE) {
    // connect to WiFi
    Serial.printf("Connecting to %s ", WIFI_SSID);
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect();

    delay(500);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println("");
    Serial.println("Trying to get NTP time...");

    getTime();

    Udp.begin(OSC_CLIENT_PORT);
    Serial.println("Starting UDP Client");
    Serial.println("");
  }
}

void loop() {
  OSCBundle bundle;

  for (int x = 0; x < NUMBER_OF_SENSORS - start; x++) {
    if (x != 0) {
      myMux.setPort(x - start);
    }
    if (IMUSensor[x]->dataAvailable() == true) {
      float quatI = IMUSensor[x]->getQuatI();
      float quatJ = IMUSensor[x]->getQuatJ();
      float quatK = IMUSensor[x]->getQuatK();
      float quatReal = IMUSensor[x]->getQuatReal();
      bundle.add(PART).add(sensornames[x]).add(quatI).add(quatJ).add(quatK).add(quatReal);
	  Serial.print(PART);
	  Serial.print(": ");
	  Serial.println(sensornames[x]);
    }
  }

  if (!WIFI_DISABLE) {
    Udp.beginPacket(OSC_HOST, OSC_HOST_PORT);
    bundle.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket();  // mark the end of the OSC Packet
  }
  bundle.empty();   // empty the bundle to free room for a new one
}
