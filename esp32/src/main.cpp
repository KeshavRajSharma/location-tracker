#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

constexpr int GPS_RX_PIN = 16;
constexpr int GPS_TX_PIN = 17;

constexpr int OLED_SDA_PIN = 21;
constexpr int OLED_SCL_PIN = 22;

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

void showWaitingScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("Live GPS Tracker");

  display.setCursor(0, 20);
  display.println("Waiting for GPS...");

  display.setCursor(0, 38);
  display.println("Move near window");

  display.display();
}

void showGpsData() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("LAT: ");
  display.println(gps.location.lat(), 6);

  display.setCursor(0, 14);
  display.print("LON: ");
  display.println(gps.location.lng(), 6);

  display.setCursor(0, 28);
  display.print("SPD: ");
  display.print(gps.speed.kmph(), 1);
  display.println(" km/h");

  display.setCursor(0, 42);
  display.print("SAT: ");
  display.println(gps.satellites.value());

  display.setCursor(0, 54);
  display.print("GPS FIXED");

  display.display();
}

void printGpsDataToSerial() {
  Serial.println("----------------------------");

  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Speed: ");
  Serial.print(gps.speed.kmph(), 1);
  Serial.println(" km/h");

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  if (gps.time.isValid()) {
    Serial.print("GPS Time: ");

    if (gps.time.hour() < 10) {
      Serial.print("0");
    }
    Serial.print(gps.time.hour());
    Serial.print(":");

    if (gps.time.minute() < 10) {
      Serial.print("0");
    }
    Serial.print(gps.time.minute());
    Serial.print(":");

    if (gps.time.second() < 10) {
      Serial.print("0");
    }
    Serial.println(gps.time.second());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  gpsSerial.begin(
      9600,
      SERIAL_8N1,
      GPS_RX_PIN,
      GPS_TX_PIN
  );

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

  if (!display.begin(
          SSD1306_SWITCHCAPVCC,
          OLED_ADDRESS
      )) {
    Serial.println("OLED initialization failed");

    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.display();

  Serial.println("ESP32 GPS and OLED test started");
  Serial.println("Waiting for GPS satellite fix...");

  showWaitingScreen();
}

void loop() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated() && gps.location.isValid()) {
    printGpsDataToSerial();
    showGpsData();
  }

  if (millis() > 10000 && gps.charsProcessed() < 10) {
    Serial.println(
        "No GPS data received. Check GPS wiring and baud rate."
    );

    delay(2000);
  }
}