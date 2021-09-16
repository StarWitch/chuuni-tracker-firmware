#include "chuuni.h"

WiFiUDP udp;
QWIICMUX i2c_muxer;
ChuuniSensor *sensorlist;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
OSCBundle bundle;

void setup() {
  // initial pixel setup
  pixel.begin();
  pixel.setBrightness(128);
  pixel.clear();
  pixel.setPixelColor(0, pixel.Color(192, 0, 192));
  pixel.show();
  delay(300);
  pixel.clear();

  // pin definitions
  pinMode(VDIV_PIN, INPUT);
  pinMode(IO_ENABLE_PIN, OUTPUT);
  pinMode(BNO_INT_RESET_PIN, OUTPUT);
  pinMode(OPT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  // hold down Boot0 pin AFTER initialization (i.e: light comes on) to get serial debug during boot
  if (SERIAL_DEBUG || !digitalRead(BOOT_BUTTON_PIN)) {
    set_serial_debug();
  }

  // interrupts for pins to set serial output
  attachInterruptArg(BOOT_BUTTON_PIN, isr, &debug_button, FALLING);
  attachInterruptArg(OPT_BUTTON_PIN, isr, &opt_button, FALLING);

  // board has two separate 3.3v regulators - one for the WiFi/MCU, and another for everything else
  // enable and turn on VCCIO 3.3v regulator, powers all IMUs (including onboard)
  digitalWrite(IO_ENABLE_PIN, HIGH);
  delay(200); // wait for stuff to power on

  // reset internal BNO085 because why not
  // IMPORTANT: reset == LOW, start == HIGH
  digitalWrite(BNO_INT_RESET_PIN, LOW);
  delay(50);
  digitalWrite(BNO_INT_RESET_PIN, HIGH);
  delay(50);

  // initialize internal and external I2C
  Wire.begin(SDA_EXT, SCL_EXT, I2C_SPEED); // sda, scl, frequency (800kHz)
  Serial.println("I2C: Began Wire");
  Wire1.begin(SDA_INT, SCL_INT, I2C_SPEED);
  Serial.println("I2C: Began Wire1");
  delay(300); // wait for i2c to truly begin

  // debugging, scans both I2C busses for everything it can see
  // this will not detect things downstream from the I2C muxer, but *will* detect the muxer itself
  if (I2C_DEBUG) {
    Serial.println("I2C: Scanning Wire");
    i2c_scanner(&Wire); // external
    Serial.println("I2C: Scanning Wire1");
    i2c_scanner(&Wire1); // internal
    if (MUX_ENABLE) {
      init_mux();
      mux_scanner(&Wire);
    }
  }

  // allocate sensor objects
  Serial.println("BNO08x: Allocating sensors");
  sensorlist = get_sensor_list();
  Serial.println("BNO08x: Allocated sensors");

  // sets up and .begin()'s all sensors'
  Serial.println("BNO08x: Starting sensors");
  init_sensors();
  Serial.println("BNO08x: Sensors started");

  // set WIFI_SSID and WIFI_PASS in your user_config.h file (see README)
  if (WIFI_DISABLE == false) {
    init_wifi();
  }

  pixel.setPixelColor(0, pixel.Color(128, 0, 128)); // violet
  pixel.show();
  pixel.setBrightness(32);
}

void loop() {
  // basic menu system to select helper functions
  // blue == calibration mode
  // green == wifi reconnect
  // red == sensor restart
  while (opt_button.pressed) {
    Serial.println("Opt: Button press received");
    opt_button.pressed = false;
    menu_selector();
    if (opt_button.pressed == false) {
      switch (opt_button.selected) {
        case 0:
          opt_button.selected = 0;
          init_sensors();
          break;
        case 1:
          opt_button.selected = 0;
          calibration_mode();
          break;
        case 2:
          opt_button.selected = 0;
          init_wifi();
          break;
        default:
          opt_button.selected = 0;
          break;
      }
      break;
    }
  }

  // enables/disables serial prints
  if (debug_button.pressed) {
    Serial.println("Debug: Button press received");
    debug_button.pressed = false;
    set_serial_debug();
  }

  for (int sensor = 0; sensor < NUMBER_OF_SENSORS; sensor++) {
    if (MUX_ENABLE) {
      if (i2c_muxer.getPort() != sensorlist[sensor].muxport) i2c_muxer.setPort(sensorlist[sensor].muxport);
    }
    /* char sensorname[50]; */
    /* snprintf(sensorname, sizeof(sensorname), "%s%s%s", PART, "/", sensorlist[sensor].name); // "/lefthand/pinkieLower", etc */

    if (sensorlist[sensor].sensor->dataAvailable()) {
      float quatI = sensorlist[sensor].sensor->getQuatI();
      float quatJ = sensorlist[sensor].sensor->getQuatJ();
      float quatK = sensorlist[sensor].sensor->getQuatK();
      float quatReal = sensorlist[sensor].sensor->getQuatReal();

      // adds the entire quaternion to an OSC bundle, with PART name and sensor name (wrist, indexUpper, etc)
      bundle.add(PART)
          .add(quatI)
          .add(quatJ)
          .add(quatK)
          .add(quatReal)
          .add(sensorlist[sensor].name); // NOTE: OscCore in Unity has problems with strings going first, so put this last

      Serial.print("BNO08x: ");
      Serial.print(sensorlist[sensor].name);
      Serial.print(" (port: ");
      Serial.print(sensorlist[sensor].muxport);
      Serial.print(")");
      Serial.print(" I: ");
      Serial.print(quatI);
      Serial.print(" J: ");
      Serial.print(quatJ);
      Serial.print(" K: ");
      Serial.print(quatK);
      Serial.print(" R: ");
      Serial.println(quatReal);
    } else {
      Serial.print("BNO08x: Data not available for: ");
      Serial.print(sensorlist[sensor].name);
      Serial.print(" ");
      Serial.println(sensor);

      pixel.clear();
      pixel.setPixelColor(0, pixel.Color(255, 0, 0));
      pixel.show();

      // send error anyways so it can be tracked in app
      float error = 0.0;
      bundle.add(PART)
        .add(error)
        .add(error)
        .add(error)
        .add(error)
        .add(sensorlist[sensor].name); // NOTE: OscCore in Unity has problems with strings going first, so put this last

      delay(5);
      pixel.clear();
      pixel.setPixelColor(0, pixel.Color(128, 0, 128));
      pixel.show();

    }
  }

  // battery level, approximately, and whether or not debug is enabled
  bundle.add(PART).add(get_battery_level()).add("battery");
  bundle.add(PART).add(debug_mode).add("debug");

  if (!WIFI_DISABLE) {
    udp.beginPacket(OSC_HOST, OSC_HOST_PORT);
    bundle.send(udp); // send the bytes to the SLIP stream
    udp.endPacket();  // mark the end of the OSC Packet
  }

  bundle.empty(); // empty the bundle to free room for a new one
}

