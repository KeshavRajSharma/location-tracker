#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

// GPS pins
constexpr int GPS_RX_PIN = 16;
constexpr int GPS_TX_PIN = 17;

// OLED pins
constexpr int OLED_SDA_PIN = 21;
constexpr int OLED_SCL_PIN = 22;

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// Timing
constexpr unsigned long WIFI_TIMEOUT_MS = 20000;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
constexpr unsigned long HEALTH_CHECK_INTERVAL_MS = 15000;

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

unsigned long lastWiFiRetryAt = 0;
unsigned long lastHealthCheckAt = 0;

bool backendOnline = false;

// --------------------------------------------------
// OLED
// --------------------------------------------------

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

  display.setCursor(0, 13);
  display.print("LON: ");
  display.println(gps.location.lng(), 6);

  display.setCursor(0, 26);
  display.print("RAW SPD: ");
  display.print(gps.speed.kmph(), 1);

  display.setCursor(0, 39);
  display.print("SAT: ");
  display.println(gps.satellites.value());

  display.setCursor(0, 52);

  if (WiFi.status() != WL_CONNECTED) {
    display.print("WiFi: Offline");
  } else if (backendOnline) {
    display.print("Backend: Online");
  } else {
    display.print("Backend: Offline");
  }

  display.display();
}

// --------------------------------------------------
// Wi-Fi
// --------------------------------------------------

bool connectToWiFi() {
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
        "GPS still works",
        "Retrying..."
    );

    return false;
  }

  Serial.println("Wi-Fi connected.");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  showMessage(
      "WiFi connected",
      WiFi.localIP().toString(),
      "Checking backend..."
  );

  return true;
}

void maintainWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  backendOnline = false;

  const unsigned long now = millis();

  if (now - lastWiFiRetryAt < WIFI_RETRY_INTERVAL_MS) {
    return;
  }

  lastWiFiRetryAt = now;

  Serial.println("Wi-Fi disconnected. Reconnecting...");

  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// --------------------------------------------------
// FastAPI health check
// --------------------------------------------------

bool checkBackendHealth() {
  if (WiFi.status() != WL_CONNECTED) {
    backendOnline = false;
    return false;
  }

  HTTPClient http;

  const String healthUrl =
      "http://" +
      String(SERVER_IP) +
      ":" +
      String(SERVER_PORT) +
      "/health";

  Serial.print("Checking backend: ");
  Serial.println(healthUrl);

  http.setConnectTimeout(5000);
  http.setTimeout(5000);
  http.begin(healthUrl);

  const int statusCode = http.GET();

  if (statusCode == 200) {
    const String response = http.getString();

    Serial.print("Backend response: ");
    Serial.println(response);

    backendOnline = true;
  } else {
    Serial.print("Backend health check failed. HTTP code: ");
    Serial.println(statusCode);

    backendOnline = false;
  }

  http.end();

  return backendOnline;
}

// --------------------------------------------------
// GPS Serial output
// --------------------------------------------------

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

  Serial.print("Backend: ");
  Serial.println(backendOnline ? "Online" : "Offline");
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

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
  checkBackendHealth();

  showMessage(
      "Tracker ready",
      WiFi.status() == WL_CONNECTED
          ? "WiFi connected"
          : "WiFi offline",
      backendOnline
          ? "Backend online"
          : "Backend offline"
  );

  delay(2000);
}

// --------------------------------------------------
// Main loop
// --------------------------------------------------

void loop() {
  maintainWiFiConnection();

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  const unsigned long now = millis();

  if (
      now - lastHealthCheckAt >=
      HEALTH_CHECK_INTERVAL_MS
  ) {
    lastHealthCheckAt = now;
    checkBackendHealth();
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
        "No GPS data received. Check wiring."
    );

    delay(2000);
  }
}