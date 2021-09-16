#include "helpers.h"

ChuuniSensor rightfingernames[] = {
  {new BNO080(), &Wire, 0, 0x4B, "thumbLower"},
  {new BNO080(), &Wire, 0, 0x4A, "thumbUpper"},
  {new BNO080(), &Wire, 1, 0x4B, "indexLower"},
  {new BNO080(), &Wire, 1, 0x4A, "indexUpper"},
  {new BNO080(), &Wire, 2, 0x4B, "middleLower"},
  {new BNO080(), &Wire, 2, 0x4A, "middleUpper"},
  {new BNO080(), &Wire, 3, 0x4B, "ringLower"},
  {new BNO080(), &Wire, 3, 0x4A, "ringUpper"},
  {new BNO080(), &Wire, 4, 0x4A, "forearm"},
  {new BNO080(), &Wire, 5, 0x4A, "wrist"},
  {new BNO080(), &Wire, 7, 0x4B, "pinkieLower"},
  {new BNO080(), &Wire, 7, 0x4A, "pinkieUpper"},
  {new BNO080(), &Wire1, 0, 0x4B, "shoulder"}, // internal, not actually using mux port 0
};

// TODO: temporary until second glove made
ChuuniSensor leftfingernames[] = {
  {new BNO080(), &Wire, 0, 0x4A, "forearm"},
  {new BNO080(), &Wire1, 0, 0x4B, "shoulder"}, // internal
};

ChuuniSensor legnames[] = {
  {new BNO080(), &Wire, 0, 0x4A, "ankle"},
  {new BNO080(), &Wire, 0, 0x4B, "foot"},
  {new BNO080(), &Wire1, 0, 0x4B, "thigh"},
};

ChuuniSensor torsonames[] = {
  {new BNO080(), &Wire, 0, 0x4A, "chest"},
  {new BNO080(), &Wire, 0, 0x4B, "head"},
  {new BNO080(), &Wire1, 0, 0x4B, "hips"},
};

ChuuniSensor defaultnames[] = {
  {new BNO080(), &Wire1, 0, 0x4B, "unassigned"},
};

// for scanning all I2C devices
void i2c_scanner(TwoWire *scan_wire) {
  byte error, address; // variable for error and I2C address
  int nDevices;

  Serial.println("I2C: Scanning");

  nDevices = 0;

  for (address = 1; address < 127; address++) {
    scan_wire->beginTransmission(address);
    error = scan_wire->endTransmission();

    if (error == 0) {
      Serial.print("I2C: Device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("I2C: Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  delay(200);

  if (nDevices == 0) {
    Serial.println("I2C: No devices found\n");
  } else {
    Serial.println("I2C: Scanning complete\n");
  }
}

// for scanning each device attached to the I2C muxer
void mux_scanner(TwoWire *mux_wire) {
  Serial.println("I2C: Scanning I2C Muxer");
  for (int i = 0; i < 8; i++) {
    Serial.print("I2C: Mux Port: ");
    Serial.println(i);
    i2c_muxer.setPort(i);
    i2c_scanner(mux_wire);
  }
  Serial.println("I2C: Mux scan complete");
}

bool mux_start = false;

void init_mux() {
  if (MUX_ENABLE && !mux_start) {
    // on "external" Wire interface
    Serial.println("I2C: Starting mux");
    if (!i2c_muxer.begin()) {
      Serial.println("I2C: Mux not detected, freezing for input");
      mux_start = false;
      while (1) {
        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(250);
        pixel.clear();
        pixel.show();
        delay(250);
      }
    }
    mux_start = true;
    Serial.println("I2C: Mux started");
    delay(300); // wait for mux to truly begin
  }
}

ChuuniSensor *get_sensor_list() {
  if (PART == "/righthand") {
    return rightfingernames;
  } else if (PART == "/lefthand") {
    return leftfingernames;
  } else if (PART == "/torso") {
    return torsonames;
  } else if (PART == "/leftleg" || PART == "/rightleg") {
    return legnames;
  }
  return defaultnames;
}

void init_sensors() {
  // reset power bus
  digitalWrite(IO_ENABLE_PIN, LOW);
  delay(250);
  digitalWrite(IO_ENABLE_PIN, HIGH);
  delay(250);
  digitalWrite(BNO_INT_RESET_PIN, LOW);
  delay(50);
  digitalWrite(BNO_INT_RESET_PIN, HIGH);
  delay(50);

  // Initialize all the sensors
  bool init_success = true;
  bool enabled;

  if (MUX_ENABLE && !mux_start) init_mux();

  for (int sensor = 0; sensor < NUMBER_OF_SENSORS; sensor++) {
    pixel.setPixelColor(0, pixel.Color(128, 128, 0));
    pixel.show();

    // sensors are organized MCU -> I2C Mux -> IMU (even (0x4A)) -> IMU (odd (0x4B))
    if (MUX_ENABLE && mux_start && i2c_muxer.getPort() != sensorlist[sensor].muxport) {
      i2c_muxer.setPort(sensorlist[sensor].muxport);
      Serial.print("I2C: Mux port ");
      Serial.println(sensorlist[sensor].muxport);
      delay(100);
    }

    Serial.print("BNO08x: Configuring ");
    Serial.println(sensorlist[sensor].name);
    enabled = sensorlist[sensor].sensor->begin(sensorlist[sensor].address, *sensorlist[sensor].wire);

    if (!enabled) {
      init_success = false;

      Serial.print("BNO08x: Sensor did not begin: ");
      Serial.println(sensor);

      pixel.setPixelColor(0, pixel.Color(128, 0, 0));
      pixel.show();
      delay(500);
      pixel.clear();
      pixel.show();
    } else {
    delay(100);

      sensorlist[sensor].sensor->enableARVRStabilizedRotationVector(IMU_UPDATE_RATE); // set update rate in hertz
      pixel.setPixelColor(0, pixel.Color(0, 128, 0));
      pixel.show();

      Serial.print("BNO08x: Configured ");
      Serial.println(sensorlist[sensor].name);
      delay(300);
      pixel.clear();
      pixel.show();
    }

    delay(100);
  }

  if (!init_success) {
    Serial.println("BNO08x: Failed to initialize, waiting for input");
    while (1) {
      pixel.setPixelColor(0, pixel.Color(128, 0, 0));
      pixel.show();
      delay(500);
      pixel.clear();
      pixel.show();
      delay(500);
      if (opt_button.pressed) {
        // allow retry by resetting opt_button selection and returning
        Serial.println("Opt: Button press received, trying again");
        opt_button.selected = 0;
        return;
      }
    }
  }
}

// calibration
// mostly taken from Sparkfun's calibration example
static void print_accuracy_level(byte accuracy_number) {
  if (accuracy_number == 0) Serial.print("Unreliable");
  else if (accuracy_number == 1) Serial.print("Low");
  else if (accuracy_number == 2) Serial.print("Medium");
  else if (accuracy_number == 3) Serial.print("High");
}

// when in this mode, press "s" over serial to "save" to the chip
void calibration_mode() {
  bool calibrated = false;

  for (int sensor = 0; sensor < NUMBER_OF_SENSORS; sensor++) {
    // sensors are organized MCU -> I2C Mux -> IMU (even (0x4A)) -> IMU (odd (0x4B))
    if (i2c_muxer.getPort() != sensorlist[sensor].muxport) i2c_muxer.setPort(sensorlist[sensor].muxport);
    delay(100); // settle down
    sensorlist[sensor].sensor->calibrateAll();
    sensorlist[sensor].sensor->enableARVRStabilizedRotationVector(100);
    sensorlist[sensor].sensor->enableMagnetometer(100);

    Serial.print("BNO08x: Calibrating ");
    Serial.println(sensorlist[sensor].name);
    calibrated = false;

    delay(1000);

    while (!calibrated) {
      if (Serial.available()) {
        byte incoming = Serial.read();

        if (incoming == 's') { // waiting for 's' key over serial
          sensorlist[sensor].sensor->saveCalibration();
          sensorlist[sensor].sensor->requestCalibrationStatus();

          int counter = 100;

          while (1) {
            if (--counter == 0) break;
            if (sensorlist[sensor].sensor->dataAvailable()) {
              if (sensorlist[sensor].sensor->calibrationComplete()) {
                Serial.print("BNO08x: Calibration stored: ");
                Serial.println(sensorlist[sensor].name);
                calibrated = true;
                delay(1000);

                break;
              }
            }
            delay (1);
          }

          if (counter == 0) {
            Serial.println("BNO08x: Failed to store calibration");
          }
        }
      }

      //Look for reports from the IMU
      if (sensorlist[sensor].sensor->dataAvailable() && !calibrated) {
        float x = sensorlist[sensor].sensor->getMagX();
        float y = sensorlist[sensor].sensor->getMagY();
        float z = sensorlist[sensor].sensor->getMagZ();
        byte accuracy = sensorlist[sensor].sensor->getMagAccuracy();

        float quatI = sensorlist[sensor].sensor->getQuatI();
        float quatJ = sensorlist[sensor].sensor->getQuatJ();
        float quatK = sensorlist[sensor].sensor->getQuatK();
        float quatReal = sensorlist[sensor].sensor->getQuatReal();
        byte sensorAccuracy = sensorlist[sensor].sensor->getQuatAccuracy();

        Serial.print("Mag: ");
        Serial.print(x, 2);
        Serial.print(",");
        Serial.print(y, 2);
        Serial.print(",");
        Serial.print(z, 2);
        Serial.print(",");
        print_accuracy_level(accuracy);
        Serial.print(",");

        Serial.print("\t");

        Serial.print("Accel: ");
        Serial.print(quatI, 2);
        Serial.print(",");
        Serial.print(quatJ, 2);
        Serial.print(",");
        Serial.print(quatK, 2);
        Serial.print(",");
        Serial.print(quatReal, 2);
        Serial.print(",");
        print_accuracy_level(sensorAccuracy);
        Serial.print(",");

        Serial.println();
      }

      // allows you to press the button again to get out of calibration mode
      if (opt_button.pressed) {
        Serial.println("Opt: Button press received, ending calibration prematurely");
        sensorlist[sensor].sensor->endCalibration();
        sensorlist[sensor].sensor->enableARVRStabilizedRotationVector(IMU_UPDATE_RATE);
        sensorlist[sensor].sensor->enableMagnetometer(IMU_UPDATE_RATE);
        return;
      }

    }

    // reset to default values
    sensorlist[sensor].sensor->endCalibration();
    sensorlist[sensor].sensor->enableARVRStabilizedRotationVector(IMU_UPDATE_RATE);
    sensorlist[sensor].sensor->enableMagnetometer(IMU_UPDATE_RATE);
  }
}
