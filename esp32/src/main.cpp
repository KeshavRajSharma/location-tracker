#include <Arduino.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

// -------------------------
// GPS configuration
// -------------------------
constexpr int GPS_RX_PIN = 16;
constexpr int GPS_TX_PIN = 17;

// -------------------------
// OLED configuration
// -------------------------
constexpr int OLED_SDA_PIN = 21;
constexpr int OLED_SCL_PIN = 22;

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// -------------------------
// Wi-Fi configuration
// -------------------------
constexpr unsigned long WIFI_TIMEOUT_MS = 20000;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000;

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

unsigned long lastWiFiRetryAt = 0;

// -------------------------
// OLED helper
// -------------------------
void showMessage(
    const String &line1,
    const String &line2 = "",
    const String &line3 = ""
) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(line1);

  display.setCursor(0, 20);
  display.println(line2);

  display.setCursor(0, 40);
  display.println(line3);

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
  display.print("RAW SPD: ");
  display.print(gps.speed.kmph(), 1);

  display.setCursor(0, 42);
  display.print("SAT: ");
  display.println(gps.satellites.value());

  display.setCursor(0, 54);

  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi: Connected");
  } else {
    display.print("WiFi: Offline");
  }

  display.display();
}

// -------------------------
// Wi-Fi functions
// -------------------------
bool connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(WIFI_SSID);

  showMessage(
      "Live GPS Tracker",
      "Connecting WiFi...",
      WIFI_SSID
  );

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  const unsigned long startedAt = millis();

  while (
      WiFi.status() != WL_CONNECTED &&
      millis() - startedAt < WIFI_TIMEOUT_MS
  ) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection failed.");

    showMessage(
        "WiFi failed",
        "GPS still working",
        "Will retry..."
    );

    return false;
  }

  Serial.println("Wi-Fi connected.");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Signal strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  showMessage(
      "WiFi connected",
      WiFi.localIP().toString(),
      "Waiting for GPS..."
  );

  delay(2000);

  return true;
}

void maintainWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  const unsigned long now = millis();

  if (now - lastWiFiRetryAt < WIFI_RETRY_INTERVAL_MS) {
    return;
  }

  lastWiFiRetryAt = now;

  Serial.println("Wi-Fi disconnected. Reconnecting...");

  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// -------------------------
// GPS Serial output
// -------------------------
void printGpsDataToSerial() {
  Serial.println("----------------------------");

  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Raw GPS speed: ");
  Serial.print(gps.speed.kmph(), 1);
  Serial.println(" km/h");

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  Serial.print("Wi-Fi: ");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
  } else {
    Serial.println("Offline");
  }
}

// -------------------------
// Setup
// -------------------------
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
    Serial.println("OLED initialization failed.");

    while (true) {
      delay(1000);
    }
  }

  display.clearDisplay();
  display.display();

  Serial.println("ESP32 tracker started.");

  connectToWiFi();

  showMessage(
      "Live GPS Tracker",
      "Waiting for GPS...",
      WiFi.status() == WL_CONNECTED
          ? "WiFi connected"
          : "WiFi offline"
  );
}

// -------------------------
// Main loop
// -------------------------
void loop() {
  maintainWiFiConnection();

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (
      gps.location.isUpdated() &&
      gps.location.isValid()
  ) {
    printGpsDataToSerial();
    showGpsData();
  }

  if (
      millis() > 10000 &&
      gps.charsProcessed() < 10
  ) {
    Serial.println(
        "No GPS data received. Check wiring and baud rate."
    );

    delay(2000);
  }
}