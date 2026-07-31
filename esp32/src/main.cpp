#include <Arduino.h>
#include <WiFi.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "api_client.h"
#include "config.h"
#include "location_filter.h"

// --------------------------------------------------
// GPS configuration
// --------------------------------------------------

constexpr int GPS_RX_PIN = 16;
constexpr int GPS_TX_PIN = 17;
constexpr unsigned long GPS_MAX_AGE_MS = 5000;

// --------------------------------------------------
// OLED configuration
// --------------------------------------------------

constexpr int OLED_SDA_PIN = 21;
constexpr int OLED_SCL_PIN = 22;

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// --------------------------------------------------
// Timing configuration
// --------------------------------------------------

constexpr unsigned long WIFI_TIMEOUT_MS = 20000;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
constexpr unsigned long HEALTH_CHECK_INTERVAL_MS = 15000;
constexpr unsigned long LOCATION_CHECK_INTERVAL_MS = 2000;

// --------------------------------------------------
// Hardware objects
// --------------------------------------------------

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);

// --------------------------------------------------
// Application state
// --------------------------------------------------

unsigned long lastWiFiRetryAt = 0;
unsigned long lastHealthCheckAt = 0;
unsigned long lastLocationCheckAt = 0;

bool backendOnline = false;
bool lastLocationSent = false;

double displayedSpeedKmph = 0.0;
double lastAcceptedDistanceMeters = 0.0;

// --------------------------------------------------
// OLED functions
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
  display.print("SPD: ");
  display.print(displayedSpeedKmph, 1);
  display.println(" km/h");

  display.setCursor(0, 39);
  display.print("SAT: ");
  display.println(gps.satellites.value());

  display.setCursor(0, 52);

  if (WiFi.status() != WL_CONNECTED) {
    display.print("WiFi: Offline");
  } else if (!backendOnline) {
    display.print("Backend: Offline");
  } else if (lastLocationSent) {
    display.print("Location: Sent");
  } else {
    display.print("Waiting movement");
  }

  display.display();
}

// --------------------------------------------------
// Wi-Fi functions
// --------------------------------------------------

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
        "GPS still works",
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

  return true;
}

void maintainWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  backendOnline = false;
  lastLocationSent = false;

  const unsigned long now = millis();

  if (
      now - lastWiFiRetryAt <
      WIFI_RETRY_INTERVAL_MS
  ) {
    return;
  }

  lastWiFiRetryAt = now;

  Serial.println("Wi-Fi disconnected. Reconnecting...");

  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// --------------------------------------------------
// GPS functions
// --------------------------------------------------

bool hasUsableGpsLocation() {
  return gps.location.isValid() &&
         gps.location.age() <= GPS_MAX_AGE_MS;
}

void printGpsDataToSerial() {
  Serial.println("----------------------------");

  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);

  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  Serial.print("Filtered speed: ");
  Serial.print(displayedSpeedKmph, 2);
  Serial.println(" km/h");

  Serial.print("Last accepted distance: ");
  Serial.print(lastAcceptedDistanceMeters, 2);
  Serial.println(" m");

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  Serial.print("Backend: ");
  Serial.println(
      backendOnline ? "Online" : "Offline"
  );
}

// --------------------------------------------------
// Location processing
// --------------------------------------------------

void processCurrentLocation() {
  if (!hasUsableGpsLocation()) {
    Serial.println(
        "Location skipped: waiting for valid GPS fix."
    );

    lastLocationSent = false;
    return;
  }

  const double latitude = gps.location.lat();
  const double longitude = gps.location.lng();

  const FilteredLocation filtered =
      filterLocation(
          latitude,
          longitude,
          millis()
      );

  if (!filtered.accepted) {
    displayedSpeedKmph = filtered.speedKmph;
    lastLocationSent = false;

    Serial.print("Location rejected or ignored. Distance: ");
    Serial.print(filtered.distanceMeters, 2);
    Serial.println(" m");

    return;
  }

  displayedSpeedKmph = filtered.speedKmph;
  lastAcceptedDistanceMeters =
      filtered.distanceMeters;

  if (!backendOnline) {
    Serial.println(
        "Accepted location not sent: backend offline."
    );

    lastLocationSent = false;
    return;
  }

  lastLocationSent = sendLocation(
      filtered.latitude,
      filtered.longitude,
      filtered.speedKmph
  );

  if (lastLocationSent) {
    Serial.println("Filtered GPS location saved.");
  } else {
    Serial.println("Filtered GPS location failed.");
  }
}

// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32 tracker starting...");

  gpsSerial.begin(
      9600,
      SERIAL_8N1,
      GPS_RX_PIN,
      GPS_TX_PIN
  );

  Wire.begin(
      OLED_SDA_PIN,
      OLED_SCL_PIN
  );

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

  resetLocationFilter();

  const bool wifiConnected = connectToWiFi();

  if (wifiConnected) {
    backendOnline = checkBackendHealth();
  }

  showMessage(
      "Live GPS Tracker",
      "Waiting for GPS...",
      backendOnline
          ? "Backend online"
          : "Backend offline"
  );
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
    backendOnline = checkBackendHealth();
  }

  if (
      now - lastLocationCheckAt >=
      LOCATION_CHECK_INTERVAL_MS
  ) {
    lastLocationCheckAt = now;
    processCurrentLocation();
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