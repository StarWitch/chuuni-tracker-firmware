#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include <OSCBundle.h>
#include <OSCMessage.h>

#include <Adafruit_NeoPixel.h>

#include "chuuni.h"

#include "helpers.h"
#include "sensors.h"

QWIICMUX myMux;
WiFiUDP Udp;
BNO080 **IMUSensor;

const char **sensornames;

Adafruit_NeoPixel pixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixel.begin();
  pixel.setBrightness(128);
  pixel.clear();
  pixel.setPixelColor(0, pixel.Color(128, 128, 128));
  pixel.show();
  delay(500);
  pixel.clear();

  if (SERIAL_DEBUG) {
    Serial.begin(115200);
  }

  // battery ADC
  pinMode(VDIV_PIN, INPUT);
  int bat = analogRead(VDIV_PIN);
  Serial.print("Battery ADC: ");
  Serial.println(bat);

  // enable IO 3.3v regulator
  pinMode(IO_ENABLE_PIN, OUTPUT);
  digitalWrite(IO_ENABLE_PIN, HIGH);
  delay(500); // wait for stuff to power on

  // reset internal BNO085
  // IMPORTANT: reset == LOW
  pinMode(BNO_INT_RST, OUTPUT);
  digitalWrite(BNO_INT_RST, LOW);
  delay(50);
  digitalWrite(BNO_INT_RST, HIGH);
  delay(50);

  sensornames = getSensorsNames();

  // initialize internal and external I2C
  Serial.println("Begin wire!");
  Wire.begin(SDA_EXT, SCL_EXT, 400000); // sda, scl, frequency (400kHz)
  Wire1.begin(SDA_INT, SCL_INT, 400000);
  Serial.println("Began wire!");
  delay(300); // wait for i2c to truly begin

  // debugging, scans both I2C busses
  if (I2C_DEBUG) {
    i2cScanner(&Wire, &Wire1);
  }

  // allocate sensors
  IMUSensor = new BNO080 *[NUMBER_OF_SENSORS];
  for (int x = 0; x < NUMBER_OF_SENSORS; x++) {
    IMUSensor[x] = new BNO080();
  }
  Serial.println("Allocated sensors!");

  // Initialize all the sensors
  bool init_success = true;
  bool enabled;

  if (!MUX_DISABLE) {
    // on "external" Wire interface
    if (myMux.begin() == false) {
      Serial.println("Mux not detected. Freezing...");
      while (1) {
        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(250);
        pixel.clear();
        pixel.show();
        delay(250);
      }
    }
    delay(500); // wait for mux to truly begin

    int mux_port = -1;
    for (int sensor = 0; sensor < NUMBER_OF_SENSORS - INTERNAL_IMU_ENABLE; sensor++) {
      pixel.setPixelColor(0, pixel.Color(128, 128, 0));
      pixel.show();

      // sensors are organized MCU -> I2C Mux -> IMU (even (0x4A)) -> IMU (odd (0x4B))
      if (sensor == 10) { // TODO: HARDWARE WORKAROUND, REMOVE ME WHEN FIXED
        mux_port++;
        Serial.print("(Workaround) Mux Port ");
        Serial.println(mux_port);
        myMux.setPort(mux_port);
        enabled = IMUSensor[sensor]->begin(0x4B, Wire);
      } else if (sensor % 2 == 0) {
        mux_port++;
        Serial.print("Mux Port ");
        Serial.println(mux_port);
        myMux.setPort(mux_port);
        enabled = IMUSensor[sensor]->begin(0x4A, Wire);
      } else {
        enabled = IMUSensor[sensor]->begin(0x4B, Wire);
      }

      delay(100);

      if (enabled == false) {
        Serial.print("Sensor ");
        Serial.print(sensor);
        Serial.println(" did not begin! Check wiring");
        init_success = false;
        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(500);
        pixel.clear();
        pixel.show();
      } else {
        IMUSensor[sensor]->enableRotationVector(IMU_UPDATE_RATE); // set update rate in hertz
        Serial.print("IMU ");
        Serial.print(sensor);
        Serial.println(" configured");
      }

      delay(100);
    }

  }

  // for hooking up an IMU without a I2C Mux (body parts, etc)
  if (EXTERNAL_IMU_ENABLE) {
    Serial.println("Configuring external sensors (without Mux)...");
    for (int sensor = 0; sensor < NUMBER_OF_SENSORS - INTERNAL_IMU_ENABLE; sensor++) {
      // MCU -> 0x4B -> 0x4A
      if (sensor % 2 == 0) {
        enabled = IMUSensor[sensor]->begin(0x4B, Wire);
      } else {
        enabled = IMUSensor[sensor]->begin(0x4A, Wire);
      }

      delay(100);

      if (enabled == false) {
        Serial.print("Sensor ");
        Serial.print(sensor);
        Serial.println(" did not begin! (Check wiring.)");
        init_success = false;
        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(500);
        pixel.clear();
        pixel.show();
      } else {
        IMUSensor[sensor]->enableRotationVector(IMU_UPDATE_RATE); // set update rate in hertz
        Serial.print("IMU ");
        Serial.print(sensor);
        Serial.println(" configured.");
      }

      delay(100);
    }
  }

  // last sensor in the stack
  if (INTERNAL_IMU_ENABLE) {
    enabled = IMUSensor[NUMBER_OF_SENSORS-1]->begin(0x4B, Wire1); // internal sensor on the WiFi board
    if (enabled == false) {
      Serial.println("Failed to start onboard IMU!");
      init_success = false;
    } else {
      IMUSensor[NUMBER_OF_SENSORS-1]->enableRotationVector(IMU_UPDATE_RATE);
      Serial.print("IMU ");
      Serial.print(NUMBER_OF_SENSORS-1);
      Serial.println(" (Onboard) configured!");
    }
  }

  if (init_success == false) {
    Serial.print("Failed to initialize. Freezing...");
    while (1) {
      pixel.setPixelColor(0, pixel.Color(128, 0, 0));
      pixel.show();
      delay(500);
      pixel.clear();
      pixel.show();
      delay(500);
    }
  }

  if (!WIFI_DISABLE) {
    // connect to WiFi
    pixel.setPixelColor(0, pixel.Color(0, 0, 128));
    pixel.show();
    Serial.printf("Connecting to %s ", WIFI_SSID);
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect();

    delay(500);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      pixel.setPixelColor(0, pixel.Color(0, 0, 128));
      pixel.show();
      delay(500);
      pixel.clear();
      pixel.show();
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println("");
    Serial.println("Trying to get NTP time...");
    pixel.setPixelColor(0, pixel.Color(0, 128, 0));
    pixel.show();

    getTime(); // ensure connection works

    Udp.begin(OSC_CLIENT_PORT);
    Serial.println("Starting UDP Client");
    Serial.println("");
  }
  pixel.setPixelColor(0, pixel.Color(128, 0, 128));
  pixel.show();
  pixel.setBrightness(32);
}

void loop() {
  OSCBundle bundle;
  int mux_port = -1;
  char partname[] = "/" PART;

  pixel.clear();
  pixel.setPixelColor(0, pixel.Color(128, 0, 128));
  pixel.show();

  for (int sensor = 0; sensor < NUMBER_OF_SENSORS; sensor++) {
    if (sensor != NUMBER_OF_SENSORS - 1 && INTERNAL_IMU_ENABLE) {
      // two sensors per port, need to count mux independently
      if (sensor % 2 == 0) {
        mux_port++;
        myMux.setPort(mux_port);
      }
    }
    if (IMUSensor[sensor]->dataAvailable() == true) {
      float quatI = IMUSensor[sensor]->getQuatI();
      float quatJ = IMUSensor[sensor]->getQuatJ();
      float quatK = IMUSensor[sensor]->getQuatK();
      float quatReal = IMUSensor[sensor]->getQuatReal();
      bundle.add(partname)
          .add(sensornames[sensor])
          .add(quatI)
          .add(quatJ)
          .add(quatK)
          .add(quatReal);
    } else {
      Serial.print("data not available for: ");
      Serial.print(sensornames[sensor]);
      Serial.print(" ");
      Serial.println(sensor);

      pixel.clear();
      pixel.setPixelColor(0, pixel.Color(255, 0, 0));
      pixel.show();
    }
    /* Serial.print(PART); */
    /* Serial.print(": "); */
    /* Serial.println(sensornames[x]); */
  }

  bundle.add("/battery").add((float)(analogRead(VDIV_PIN) / 1536.0));

  if (!WIFI_DISABLE) {
    Udp.beginPacket(OSC_HOST, OSC_HOST_PORT);
    bundle.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket();  // mark the end of the OSC Packet
  }
  bundle.empty(); // empty the bundle to free room for a new one
}
