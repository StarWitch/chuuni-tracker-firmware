#include "helpers.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

static void print_local_time() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("WiFi: Failed to obtain time from NTP server");
    return;
  }
  Serial.print("WiFi: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void get_time() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  print_local_time();
}

void get_wifi() {
  // connect to WiFi
  pixel.setPixelColor(0, pixel.Color(0, 0, 128));
  pixel.show();

  Serial.printf("WiFi: Connecting to %s ", WIFI_SSID);

  WiFi.mode(WIFI_OFF);
  WiFi.disconnect();

  delay(500);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    // blink while waiting for connection
    pixel.setPixelColor(0, pixel.Color(0, 0, 128)); // blue
    pixel.show();
    delay(500);
    pixel.clear();
    pixel.show();
    delay(500);

    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi: Connected");
  Serial.print("WiFi: IP address = ");
  Serial.println(WiFi.localIP());
  Serial.println("WiFi: Attempting to retrieve NTP time");

  pixel.setPixelColor(0, pixel.Color(0, 128, 0));
  pixel.show();

  get_time(); // ensure connection works

  Serial.print("WiFi: Starting UDP Client (host: ");
  Serial.print(OSC_HOST);
  Serial.print(", port: ");
  Serial.print(OSC_HOST_PORT);
  Serial.println(")");

  // begin sending packets
  udp.begin(OSC_CLIENT_PORT);
}

// button interrupt section
Button opt_button = {OPT_BUTTON_PIN, 0, false};
Button debug_button = {BOOT_BUTTON_PIN, 0, false};

void ARDUINO_ISR_ATTR isr(void *arg) {
    Button* s = static_cast<Button*>(arg);
    if (s->selected < 2) {
      s->selected += 1;
    } else {
      s->selected = 0;
    }
    s->pressed = true;
}

void menu_selector() {
  switch (opt_button.selected) {
    case 0:
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(128, 0, 0));
      pixel.show();
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(128, 0, 0));
      pixel.show();
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(128, 0, 0));
      pixel.show();
      delay(1000);
      break;
    case 1:
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 128, 0));
      pixel.show();
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 128, 0));
      pixel.show();
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 128, 0));
      pixel.show();
      delay(1000);
      break;
    case 2:
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 128));
      pixel.show();
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 128));
      pixel.show();
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      delay(100);
      pixel.setPixelColor(0, pixel.Color(0, 0, 128));
      pixel.show();
      delay(1000);
      break;
    default:
      break;
  }
}

float get_battery_level() {
  float voltage = analogRead(VDIV_PIN) / VOLTAGE_READ_CONSTANT;
  return voltage;
}

void show_serial_splash() {
  Serial.println("ChuuniTracker Firmware (alpha)");
  Serial.println("Author: Alice D. (StarWitch Productions)");
  Serial.println("\n-- Settings --");
  Serial.println("* Functions *");
  Serial.print("WiFi Disabled: ");
  Serial.println(WIFI_DISABLE);
  Serial.print("I2C Mux Disabled: ");
  Serial.println(MUX_DISABLE);
  Serial.print("Internal IMU: ");
  Serial.println(INTERNAL_IMU_ENABLE);
  Serial.print("External IMU: ");
  Serial.println(EXTERNAL_IMU_ENABLE);
  Serial.println("\n* Debug *");
  Serial.print("I2C Debug: ");
  Serial.println(I2C_DEBUG);
  Serial.println("\n* Pin Definitions *");
  Serial.print("Internal I2C: ");
  Serial.print(SDA_INT);
  Serial.print(" (SDA) / ");
  Serial.print(SCL_INT);
  Serial.println(" (SCL)");
  Serial.print("External I2C: ");
  Serial.print(SDA_EXT);
  Serial.print(" (SDA) / ");
  Serial.print(SCL_EXT);
  Serial.println(" (SCL)");
  Serial.print("VCCIO 3.3v Enable Pin: ");
  Serial.println(IO_ENABLE_PIN);
  Serial.print("Internal IMU Reset Pin: ");
  Serial.println(BNO_INT_RESET_PIN);
  Serial.print("NeoPixel Status LED Pin: ");
  Serial.println(NEOPIXEL_PIN);
  Serial.print("Voltage Divider ADC Pin: ");
  Serial.println(VDIV_PIN);
  Serial.print("Opt Button Pin: ");
  Serial.println(OPT_BUTTON_PIN);
  Serial.println("\n* Network *");
  Serial.print("WiFi SSID: ");
  Serial.println(WIFI_SSID);
  Serial.print("WiFi Password: ");
  Serial.println("***********");
  Serial.print("OSC Receiving Host/Port: ");
  Serial.print(OSC_HOST);
  Serial.print(":");
  Serial.println(OSC_HOST_PORT);
  Serial.print("OSC Client Port (this board): ");
  Serial.println(OSC_CLIENT_PORT);
  Serial.println("\n* Part *");
  Serial.print("Part: ");
  Serial.println(PART);
  Serial.print("# of Sensors: ");
  Serial.println(NUMBER_OF_SENSORS);
  Serial.print("Battery voltage (approx): ");
  Serial.println(get_battery_level());
}

bool debug_mode;

void set_serial_debug() {
  if (debug_mode) {
    Serial.println("MCU: Serial debug mode disabled!");
    Serial.end();
    debug_mode = false;
  } else {
    Serial.begin(115200);
    show_serial_splash();
    Serial.println("MCU: Serial/Debug Mode enabled");
    Serial.println("MCU: (CAUTION) This significantly slows down reading from the IMUs so only enable this during debugging!");
    delay(500);

    debug_mode = true;
  }
}
