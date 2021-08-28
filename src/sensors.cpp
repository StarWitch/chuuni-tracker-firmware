#include "helpers.h"

const char *fingernames[] = {
    "pinkieUpper", "pinkieLower", "ringUpper", "ringLower",
    "middleUpper", "middleLower", "indexUpper", "indexLower",
    "thumbUpper",  "thumbLower", "wrist", "forearm"
};

const char *torsonames[] = {
    "leftArm"
    "head",
    "upperBack",
    "rightArm",
    "lowerBack",
};

const char *legnames[] = {
    "thigh",
    "calf",
    "foot",
};

const char *defaultnames[] = {
    "sensor",
};

const char **get_sensor_names() {
  if (PART == "/lefthand" || PART == "/righthand") {
    return fingernames;
  } else if (PART == "/leftleg" || PART == "/rightleg") {
    return legnames;
  } else if (PART == "/torso") {
    return torsonames;
  } else {
    Serial.println("MCU: PART not defined!");
    return defaultnames;
  }
}

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

bool mux_start = false;
void get_sensors() {
  // Initialize all the sensors
  bool init_success = true;
  bool enabled;

  if (!MUX_DISABLE && !mux_start) {
    // on "external" Wire interface
    if (i2c_muxer.begin() == false) {
      Serial.println("I2C: Mux not detected, freezing for input");
      while (1) {
        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(250);
        pixel.clear();
        pixel.show();
        delay(250);
      }
      mux_start = true;
      delay(300); // wait for mux to truly begin
    }

    int mux_port = -1;
    for (int sensor = 0; sensor < NUMBER_OF_SENSORS - INTERNAL_IMU_ENABLE; sensor++) {
      pixel.setPixelColor(0, pixel.Color(128, 128, 0));
      pixel.show();

      // sensors are organized MCU -> I2C Mux -> IMU (even (0x4A)) -> IMU (odd (0x4B))
      if (sensor == 10) { // TODO: HARDWARE WORKAROUND, REMOVE ME WHEN FIXED
        mux_port++;
        i2c_muxer.setPort(mux_port);
        Serial.print("(Workaround) Mux Port ");
        Serial.println(mux_port);

        enabled = imu_sensors[sensor]->begin(0x4B, Wire);
      } else if (sensor % 2 == 0) {
        mux_port++;
        i2c_muxer.setPort(mux_port);
        Serial.print("I2C: Mux Port ");
        Serial.println(mux_port);

        enabled = imu_sensors[sensor]->begin(0x4A, Wire);
      } else {
        enabled = imu_sensors[sensor]->begin(0x4B, Wire);
      }

      if (enabled == false) {
        init_success = false;

        Serial.print("BNO08x: Sensor ");
        Serial.print(sensor);
        Serial.println(" did not begin!");

        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(500);
        pixel.clear();
        pixel.show();
      } else {
        imu_sensors[sensor]->enableRotationVector(IMU_UPDATE_RATE); // set update rate in hertz

        Serial.print("BNO08x: ");
        Serial.print(sensor);
        Serial.println(" configured");
      }

      delay(100);
    }

  }

  // for hooking up an IMU without a I2C Mux (body parts, etc)
  if (EXTERNAL_IMU_ENABLE) {
    Serial.println("BNO08x: Configuring non-muxed sensors");
    for (int sensor = 0; sensor < NUMBER_OF_SENSORS - INTERNAL_IMU_ENABLE; sensor++) {
      // MCU -> 0x4B -> 0x4A
      if (sensor % 2 == 0) {
        enabled = imu_sensors[sensor]->begin(0x4B, Wire);
      } else {
        enabled = imu_sensors[sensor]->begin(0x4A, Wire);
      }

      if (enabled == false) {
        Serial.print("I2c: Sensor ");
        Serial.print(sensor);
        Serial.println(" did not begin!");
        init_success = false;
        pixel.setPixelColor(0, pixel.Color(128, 0, 0));
        pixel.show();
        delay(500);
        pixel.clear();
        pixel.show();
      } else {
        imu_sensors[sensor]->enableRotationVector(IMU_UPDATE_RATE); // set update rate in hertz
        Serial.print("BNO08x: ");
        Serial.print(sensor);
        Serial.println(" configured.");
      }

      delay(20);
    }
  }

  // last sensor in the stack
  if (INTERNAL_IMU_ENABLE) {
    enabled = imu_sensors[NUMBER_OF_SENSORS-1]->begin(0x4B, Wire1); // internal sensor on the WiFi board
    if (enabled == false) {
      Serial.println("BNO08x: Failed to start onboard IMU");
      init_success = false;
    } else {
      imu_sensors[NUMBER_OF_SENSORS-1]->enableRotationVector(IMU_UPDATE_RATE);
      Serial.print("BNO08x: ");
      Serial.print(NUMBER_OF_SENSORS-1);
      Serial.println(" (Onboard) configured!");
    }
  }

  if (init_success == false) {
    Serial.print("BNO08x: Failed to initialize, waiting for input");
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
  int mux_port = -1;
  bool calibrated = false;

  for (int sensor = 0; sensor < NUMBER_OF_SENSORS; sensor++) {
    // sensors are organized MCU -> I2C Mux -> IMU (even (0x4A)) -> IMU (odd (0x4B))
    if (sensor == 10) { // TODO: HARDWARE WORKAROUND, REMOVE ME WHEN FIXED
      mux_port++;
      Serial.print("(Workaround) Mux Port ");
      Serial.println(mux_port);
      i2c_muxer.setPort(mux_port);
    } else if (sensor % 2 == 0) {
      mux_port++;
      Serial.print("I2C: Mux Port ");
      Serial.println(mux_port);
      i2c_muxer.setPort(mux_port);
    }

    imu_sensors[sensor]->calibrateAll();
    imu_sensors[sensor]->enableGameRotationVector(100);
    imu_sensors[sensor]->enableMagnetometer(100);

    Serial.print("BNO08x: Calibrating ");
    Serial.println(sensornames[sensor]);
    calibrated = false;

    delay(1000);

    while (calibrated == false) {
      if (Serial.available()) {
        byte incoming = Serial.read();

        if (incoming == 's') {
          imu_sensors[sensor]->saveCalibration();
          imu_sensors[sensor]->requestCalibrationStatus();

          int counter = 100;

          while (1) {
            if (--counter == 0) break;
            if (imu_sensors[sensor]->dataAvailable() == true) {
              if (imu_sensors[sensor]->calibrationComplete() == true) {
                Serial.print("BNO08x: ");
                Serial.print(sensornames[sensor]);
                Serial.println(" calibration stored");
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
      if (imu_sensors[sensor]->dataAvailable() == true && calibrated == false) {
        float x = imu_sensors[sensor]->getMagX();
        float y = imu_sensors[sensor]->getMagY();
        float z = imu_sensors[sensor]->getMagZ();
        byte accuracy = imu_sensors[sensor]->getMagAccuracy();

        float quatI = imu_sensors[sensor]->getQuatI();
        float quatJ = imu_sensors[sensor]->getQuatJ();
        float quatK = imu_sensors[sensor]->getQuatK();
        float quatReal = imu_sensors[sensor]->getQuatReal();
        byte sensorAccuracy = imu_sensors[sensor]->getQuatAccuracy();

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
        imu_sensors[sensor]->endCalibration();
        imu_sensors[sensor]->enableGameRotationVector(IMU_UPDATE_RATE);
        imu_sensors[sensor]->enableMagnetometer(IMU_UPDATE_RATE);
        return;
      }

    }

    // reset to default values
    imu_sensors[sensor]->endCalibration();
    imu_sensors[sensor]->enableGameRotationVector(IMU_UPDATE_RATE);
    imu_sensors[sensor]->enableMagnetometer(IMU_UPDATE_RATE);
  }
}
