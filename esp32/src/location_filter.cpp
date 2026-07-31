#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "location_filter.h"

namespace {

constexpr double MIN_MOVEMENT_METERS = 2.5;
constexpr double MAX_WALKING_SPEED_KMPH = 6.0;
constexpr double SPEED_SMOOTHING_FACTOR = 0.20;

constexpr int REQUIRED_MOVEMENT_READINGS = 3;

bool hasPreviousLocation = false;

double previousLatitude = 0.0;
double previousLongitude = 0.0;

unsigned long previousTimestampMs = 0;

double smoothedSpeedKmph = 0.0;

int consecutiveMovementReadings = 0;

bool isValidCoordinate(
    const double latitude,
    const double longitude
) {
  return latitude >= -90.0 &&
         latitude <= 90.0 &&
         longitude >= -180.0 &&
         longitude <= 180.0;
}

double calculateSpeedKmph(
    const double distanceMeters,
    const unsigned long elapsedMs
) {
  if (elapsedMs == 0) {
    return 0.0;
  }

  const double elapsedSeconds =
      static_cast<double>(elapsedMs) / 1000.0;

  return (distanceMeters / elapsedSeconds) * 3.6;
}

double smoothSpeed(const double newSpeedKmph) {
  smoothedSpeedKmph =
      SPEED_SMOOTHING_FACTOR * newSpeedKmph +
      (1.0 - SPEED_SMOOTHING_FACTOR) *
          smoothedSpeedKmph;

  return smoothedSpeedKmph;
}

}  // namespace

void resetLocationFilter() {
  hasPreviousLocation = false;

  previousLatitude = 0.0;
  previousLongitude = 0.0;
  previousTimestampMs = 0;

  smoothedSpeedKmph = 0.0;
  consecutiveMovementReadings = 0;
}

FilteredLocation filterLocation(
    const double latitude,
    const double longitude,
    const unsigned long timestampMs
) {
  FilteredLocation result{
      latitude,
      longitude,
      0.0,
      0.0,
      false
  };

  if (!isValidCoordinate(latitude, longitude)) {
    return result;
  }

  // Accept the first valid point as the starting location.
  if (!hasPreviousLocation) {
    previousLatitude = latitude;
    previousLongitude = longitude;
    previousTimestampMs = timestampMs;

    hasPreviousLocation = true;

    result.accepted = true;
    return result;
  }

  const double distanceMeters =
      TinyGPSPlus::distanceBetween(
          previousLatitude,
          previousLongitude,
          latitude,
          longitude
      );

  const unsigned long elapsedMs =
      timestampMs - previousTimestampMs;

  result.distanceMeters = distanceMeters;

  // Treat small coordinate changes as stationary GPS noise.
  if (distanceMeters < MIN_MOVEMENT_METERS) {
    consecutiveMovementReadings = 0;
    smoothedSpeedKmph = 0.0;

    result.speedKmph = 0.0;
    return result;
  }

  const double calculatedSpeedKmph =
      calculateSpeedKmph(
          distanceMeters,
          elapsedMs
      );

  // Reject unrealistic jumps.
  if (
      calculatedSpeedKmph <= 0.0 ||
      calculatedSpeedKmph > MAX_WALKING_SPEED_KMPH
  ) {
    consecutiveMovementReadings = 0;
    smoothedSpeedKmph = 0.0;

    result.speedKmph = 0.0;
    return result;
  }

  consecutiveMovementReadings++;

  Serial.print("Movement confirmation: ");
  Serial.print(consecutiveMovementReadings);
  Serial.print("/");
  Serial.println(REQUIRED_MOVEMENT_READINGS);

  // Do not show speed until movement is detected continuously.
  if (
      consecutiveMovementReadings <
      REQUIRED_MOVEMENT_READINGS
  ) {
    result.speedKmph = 0.0;
    return result;
  }

  result.speedKmph =
      smoothSpeed(calculatedSpeedKmph);

  result.accepted = true;

  previousLatitude = latitude;
  previousLongitude = longitude;
  previousTimestampMs = timestampMs;

  consecutiveMovementReadings = 0;

  return result;
}