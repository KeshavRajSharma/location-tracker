#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "api_client.h"
#include "config.h"

namespace {

constexpr int HTTP_TIMEOUT_MS = 5000;

String buildServerUrl(const char *path) {
  return String("http://") +
         SERVER_IP +
         ":" +
         SERVER_PORT +
         path;
}

bool isSuccessfulStatus(const int statusCode) {
  return statusCode >= 200 && statusCode < 300;
}

bool isValidCoordinate(
    const double latitude,
    const double longitude
) {
  return latitude >= -90.0 &&
         latitude <= 90.0 &&
         longitude >= -180.0 &&
         longitude <= 180.0;
}

}  // namespace

bool checkBackendHealth() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Health check skipped: WiFi offline.");
    return false;
  }

  HTTPClient http;
  const String url = buildServerUrl("/health");

  Serial.print("Checking backend: ");
  Serial.println(url);

  http.setConnectTimeout(HTTP_TIMEOUT_MS);
  http.setTimeout(HTTP_TIMEOUT_MS);

  if (!http.begin(url)) {
    Serial.println("Failed to initialize health request.");
    return false;
  }

  const int statusCode = http.GET();

  Serial.print("Health HTTP status: ");
  Serial.println(statusCode);

  const bool backendHealthy =
      statusCode == HTTP_CODE_OK;

  if (backendHealthy) {
    const String response = http.getString();

    Serial.print("Backend response: ");
    Serial.println(response);
  } else {
    Serial.print("Health check failed: ");
    Serial.println(http.errorToString(statusCode));
  }

  http.end();

  return backendHealthy;
}

bool sendLocation(
    const double latitude,
    const double longitude,
    const double speedKmph
) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Location not sent: WiFi offline.");
    return false;
  }

  if (!isValidCoordinate(latitude, longitude)) {
    Serial.println("Location not sent: invalid coordinates.");
    return false;
  }

  HTTPClient http;
  const String url = buildServerUrl("/locations");

  JsonDocument document;

  document["device_id"] = DEVICE_ID;
  document["latitude"] = latitude;
  document["longitude"] = longitude;
  document["speed"] = speedKmph >= 0.0
                          ? speedKmph
                          : 0.0;

  String requestBody;
  serializeJson(document, requestBody);

  Serial.print("POST ");
  Serial.println(url);

  Serial.print("Payload: ");
  Serial.println(requestBody);

  http.setConnectTimeout(HTTP_TIMEOUT_MS);
  http.setTimeout(HTTP_TIMEOUT_MS);

  if (!http.begin(url)) {
    Serial.println("Failed to initialize location request.");
    return false;
  }

  http.addHeader(
      "Content-Type",
      "application/json"
  );

  const int statusCode = http.POST(requestBody);
  const String response = http.getString();

  Serial.print("Location HTTP status: ");
  Serial.println(statusCode);

  if (!response.isEmpty()) {
    Serial.print("Server response: ");
    Serial.println(response);
  }

  const bool locationSaved =
      isSuccessfulStatus(statusCode);

  if (!locationSaved) {
    Serial.print("Location request failed: ");
    Serial.println(http.errorToString(statusCode));
  }

  http.end();

  return locationSaved;
}