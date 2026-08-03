#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "location_filter.h"

namespace {

/*
 * Movement must reach this distance before the tracker
 * begins considering the device as moving.
 */
constexpr double START_MOVEMENT_METERS = 2.0;

/*
 * Once movement is confirmed, a new route point is accepted
 * after approximately this much additional movement.
 */
constexpr double TRACK_POINT_METERS = 1.5;

/*
 * Distance below this value can help confirm that
 * the tracker has stopped.
 */
constexpr double STOP_DISTANCE_METERS = 1.0;

/*
 * Minimum useful movement speed.
 * Lower values are treated as stationary noise.
 */
constexpr double MIN_MOVING_SPEED_KMPH = 0.5;

/*
 * Used to confirm that the device has stopped.
 */
constexpr double STOP_SPEED_KMPH = 0.4;

/*
 * Maximum valid speed for this walking tracker.
 */
constexpr double MAX_VALID_SPEED_KMPH = 12.0;

/*
 * Reject very large coordinate jumps.
 */
constexpr double MAX_POSITION_JUMP_KMPH = 20.0;

/*
 * Higher values react faster.
 * Lower values produce smoother results.
 */
constexpr double SPEED_SMOOTHING_FACTOR = 0.35;

/*
 * Number of continuous movement readings needed
 * before entering the moving state.
 */
constexpr int START_CONFIRMATION_READINGS = 2;

/*
 * Number of low-movement readings needed
 * before returning to the stationary state.
 */
constexpr int STOP_CONFIRMATION_READINGS = 3;

/*
 * Basic GPS-quality requirements.
 */
constexpr unsigned int MIN_SATELLITES = 4;
constexpr double MAX_HDOP = 6.0;

// --------------------------------------------------
// Filter state
// --------------------------------------------------

bool hasReferenceLocation = false;
bool isMovingState = false;

double referenceLatitude = 0.0;
double referenceLongitude = 0.0;

unsigned long referenceTimestampMs = 0;

double smoothedSpeedKmph = 0.0;

int movementConfirmationCount = 0;
int stopConfirmationCount = 0;

// --------------------------------------------------
// Helper functions
// --------------------------------------------------

bool isValidCoordinate(
    const double latitude,
    const double longitude
) {
  return latitude >= -90.0 &&
         latitude <= 90.0 &&
         longitude >= -180.0 &&
         longitude <= 180.0;
}

bool hasAcceptableGpsQuality(
    const unsigned int satellites,
    const double hdop,
    const bool hdopValid
) {
  if (satellites < MIN_SATELLITES) {
    return false;
  }

  if (hdopValid && hdop > MAX_HDOP) {
    return false;
  }

  return true;
}

double calculatePositionSpeedKmph(
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

double chooseSpeedKmph(
    const double gpsSpeedKmph,
    const bool gpsSpeedValid,
    const double positionSpeedKmph
) {
  /*
   * Prefer the NEO-6M speed-over-ground value when it
   * is valid and inside the expected walking range.
   */
  if (
      gpsSpeedValid &&
      gpsSpeedKmph >= MIN_MOVING_SPEED_KMPH &&
      gpsSpeedKmph <= MAX_VALID_SPEED_KMPH
  ) {
    return gpsSpeedKmph;
  }

  /*
   * Otherwise calculate speed from distance and time.
   */
  return positionSpeedKmph;
}

double smoothSpeed(const double newSpeedKmph) {
  if (smoothedSpeedKmph <= 0.0) {
    smoothedSpeedKmph = newSpeedKmph;
    return smoothedSpeedKmph;
  }

  smoothedSpeedKmph =
      SPEED_SMOOTHING_FACTOR * newSpeedKmph +
      (1.0 - SPEED_SMOOTHING_FACTOR) *
          smoothedSpeedKmph;

  return smoothedSpeedKmph;
}

void updateReferenceLocation(
    const double latitude,
    const double longitude,
    const unsigned long timestampMs
) {
  referenceLatitude = latitude;
  referenceLongitude = longitude;
  referenceTimestampMs = timestampMs;
}

}  // namespace

// --------------------------------------------------
// Public functions
// --------------------------------------------------

void resetLocationFilter() {
  hasReferenceLocation = false;
  isMovingState = false;

  referenceLatitude = 0.0;
  referenceLongitude = 0.0;
  referenceTimestampMs = 0;

  smoothedSpeedKmph = 0.0;

  movementConfirmationCount = 0;
  stopConfirmationCount = 0;
}

FilteredLocation filterLocation(
    const double latitude,
    const double longitude,
    const double gpsSpeedKmph,
    const bool gpsSpeedValid,
    const unsigned int satellites,
    const double hdop,
    const bool hdopValid,
    const unsigned long timestampMs
) {
  FilteredLocation result{
      latitude,
      longitude,
      isMovingState ? smoothedSpeedKmph : 0.0,
      0.0,
      false,
      !isMovingState,
      isMovingState,
      false
  };

  if (!isValidCoordinate(latitude, longitude)) {
    Serial.println("Filter rejected: invalid coordinate.");
    return result;
  }

  result.qualityAccepted =
      hasAcceptableGpsQuality(
          satellites,
          hdop,
          hdopValid
      );

  if (!result.qualityAccepted) {
    Serial.print("Filter rejected: weak GPS quality. SAT=");
    Serial.print(satellites);

    if (hdopValid) {
      Serial.print(", HDOP=");
      Serial.println(hdop, 2);
    } else {
      Serial.println(", HDOP unavailable");
    }

    return result;
  }

  /*
   * First reliable GPS point becomes the reference point.
   */
  if (!hasReferenceLocation) {
    updateReferenceLocation(
        latitude,
        longitude,
        timestampMs
    );

    hasReferenceLocation = true;

    result.speedKmph = 0.0;
    result.accepted = true;
    result.stationary = true;
    result.moving = false;

    Serial.println("First GPS location accepted.");

    return result;
  }

  const double distanceMeters =
      TinyGPSPlus::distanceBetween(
          referenceLatitude,
          referenceLongitude,
          latitude,
          longitude
      );

  const unsigned long elapsedMs =
      timestampMs - referenceTimestampMs;

  const double positionSpeedKmph =
      calculatePositionSpeedKmph(
          distanceMeters,
          elapsedMs
      );

  result.distanceMeters = distanceMeters;

  /*
   * Large coordinate jumps are treated as GPS errors.
   */
  if (positionSpeedKmph > MAX_POSITION_JUMP_KMPH) {
    movementConfirmationCount = 0;

    Serial.print("GPS jump rejected. Position speed: ");
    Serial.print(positionSpeedKmph, 2);
    Serial.println(" km/h");

    return result;
  }

  const double selectedSpeedKmph =
      chooseSpeedKmph(
          gpsSpeedKmph,
          gpsSpeedValid,
          positionSpeedKmph
      );

  // --------------------------------------------------
  // Stationary state
  // --------------------------------------------------

  if (!isMovingState) {
    const bool distanceShowsMovement =
        distanceMeters >= START_MOVEMENT_METERS;

    const bool speedShowsMovement =
        selectedSpeedKmph >= MIN_MOVING_SPEED_KMPH &&
        selectedSpeedKmph <= MAX_VALID_SPEED_KMPH;

    if (distanceShowsMovement && speedShowsMovement) {
      movementConfirmationCount++;

      Serial.print("Movement confirmation: ");
      Serial.print(movementConfirmationCount);
      Serial.print("/");
      Serial.println(START_CONFIRMATION_READINGS);
    } else {
      movementConfirmationCount = 0;
      smoothedSpeedKmph = 0.0;

      result.speedKmph = 0.0;
      result.stationary = true;
      result.moving = false;

      return result;
    }

    if (
        movementConfirmationCount <
        START_CONFIRMATION_READINGS
    ) {
      return result;
    }

    /*
     * Movement is now confirmed.
     */
    isMovingState = true;

    movementConfirmationCount = 0;
    stopConfirmationCount = 0;

    smoothedSpeedKmph = selectedSpeedKmph;

    result.speedKmph = smoothedSpeedKmph;
    result.accepted = true;
    result.stationary = false;
    result.moving = true;

    updateReferenceLocation(
        latitude,
        longitude,
        timestampMs
    );

    Serial.println("Movement state started.");

    return result;
  }

  // --------------------------------------------------
  // Moving state
  // --------------------------------------------------

  const bool lowDistance =
      distanceMeters < STOP_DISTANCE_METERS;

  const bool lowGpsSpeed =
      !gpsSpeedValid ||
      gpsSpeedKmph < STOP_SPEED_KMPH;

  const bool lowPositionSpeed =
      positionSpeedKmph < MIN_MOVING_SPEED_KMPH;

  /*
   * Confirm stopping using several continuous readings.
   */
  if (
      lowDistance &&
      lowGpsSpeed &&
      lowPositionSpeed
  ) {
    stopConfirmationCount++;

    Serial.print("Stop confirmation: ");
    Serial.print(stopConfirmationCount);
    Serial.print("/");
    Serial.println(STOP_CONFIRMATION_READINGS);
  } else {
    stopConfirmationCount = 0;
  }

  if (
      stopConfirmationCount >=
      STOP_CONFIRMATION_READINGS
  ) {
    isMovingState = false;

    movementConfirmationCount = 0;
    stopConfirmationCount = 0;
    smoothedSpeedKmph = 0.0;

    /*
     * Use the final accepted coordinate rather than a
     * drifting stationary coordinate.
     *
     * This sends one 0 km/h update to the backend.
     */
    result.latitude = referenceLatitude;
    result.longitude = referenceLongitude;
    result.distanceMeters = 0.0;
    result.speedKmph = 0.0;

    result.accepted = true;
    result.stationary = true;
    result.moving = false;

    Serial.println("Movement stopped. Sending 0.0 km/h.");

    return result;
  }

  /*
   * While moving, wait until there is enough distance
   * for another meaningful route point.
   */
  if (distanceMeters < TRACK_POINT_METERS) {
    result.speedKmph = smoothedSpeedKmph;
    result.stationary = false;
    result.moving = true;

    return result;
  }

  if (
      selectedSpeedKmph <= 0.0 ||
      selectedSpeedKmph > MAX_VALID_SPEED_KMPH
  ) {
    Serial.print("Moving point rejected. Speed: ");
    Serial.print(selectedSpeedKmph, 2);
    Serial.println(" km/h");

    return result;
  }

  result.speedKmph =
      smoothSpeed(selectedSpeedKmph);

  result.accepted = true;
  result.stationary = false;
  result.moving = true;

  updateReferenceLocation(
      latitude,
      longitude,
      timestampMs
  );

  Serial.print("Moving point accepted. Distance: ");
  Serial.print(distanceMeters, 2);
  Serial.print(" m, Speed: ");
  Serial.print(result.speedKmph, 2);
  Serial.println(" km/h");

  return result;
}