#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "location_filter.h"

namespace {

// --------------------------------------------------
// Balanced settings for a walking/corridor demonstration
// --------------------------------------------------

/*
 * Distance from the last accepted route point required
 * before starting to confirm movement.
 */
constexpr double START_MOVEMENT_METERS = 2.5;

/*
 * Distance from the last accepted route point required
 * before sending another route point.
 */
constexpr double TRACK_POINT_METERS = 1.5;

/*
 * Consecutive-fix distance below this value helps confirm
 * that the tracker has stopped.
 */
constexpr double STOP_DISTANCE_METERS = 1.2;

/*
 * Minimum useful moving speed.
 */
constexpr double MIN_MOVING_SPEED_KMPH = 0.4;

/*
 * GPS speed below this value helps confirm stopping.
 */
constexpr double STOP_SPEED_KMPH = 0.3;

/*
 * Maximum accepted walking/running speed.
 */
constexpr double MAX_VALID_SPEED_KMPH = 12.0;

/*
 * Position-speed values above this limit are treated
 * as sudden GPS jumps.
 */
constexpr double MAX_POSITION_JUMP_KMPH = 25.0;

/*
 * 30% new speed and 70% previous smoothed speed.
 */
constexpr double SPEED_SMOOTHING_FACTOR = 0.30;

/*
 * Two continuous movement readings are required
 * before entering the moving state.
 */
constexpr int START_CONFIRMATION_READINGS = 2;

/*
 * Four continuous stationary readings are required
 * before stopping.
 */
constexpr int STOP_CONFIRMATION_READINGS = 4;

/*
 * Permissive GPS-quality limits for classroom testing.
 */
constexpr unsigned int MIN_SATELLITES = 4;
constexpr double MAX_HDOP = 8.0;

// --------------------------------------------------
// Filter state
// --------------------------------------------------

bool hasRouteReference = false;
bool hasPreviousFix = false;
bool isMovingState = false;

/*
 * Last point accepted and sent as part of the route.
 */
double routeReferenceLatitude = 0.0;
double routeReferenceLongitude = 0.0;

/*
 * Previous valid GPS fix.
 * Used only for consecutive-fix speed calculation.
 */
double previousFixLatitude = 0.0;
double previousFixLongitude = 0.0;
unsigned long previousFixTimestampMs = 0;

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
   * is inside the accepted range.
   */
  if (
      gpsSpeedValid &&
      gpsSpeedKmph >= MIN_MOVING_SPEED_KMPH &&
      gpsSpeedKmph <= MAX_VALID_SPEED_KMPH
  ) {
    return gpsSpeedKmph;
  }

  /*
   * Otherwise use speed calculated from two
   * consecutive GPS fixes.
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

void updateRouteReference(
    const double latitude,
    const double longitude
) {
  routeReferenceLatitude = latitude;
  routeReferenceLongitude = longitude;
}

void updatePreviousFix(
    const double latitude,
    const double longitude,
    const unsigned long timestampMs
) {
  previousFixLatitude = latitude;
  previousFixLongitude = longitude;
  previousFixTimestampMs = timestampMs;
}

}  // namespace

// --------------------------------------------------
// Public functions
// --------------------------------------------------

void resetLocationFilter() {
  hasRouteReference = false;
  hasPreviousFix = false;
  isMovingState = false;

  routeReferenceLatitude = 0.0;
  routeReferenceLongitude = 0.0;

  previousFixLatitude = 0.0;
  previousFixLongitude = 0.0;
  previousFixTimestampMs = 0;

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
    Serial.print("Weak GPS quality. SAT=");
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
   * First reliable GPS fix initializes both:
   * 1. route reference
   * 2. previous-fix reference
   */
  if (!hasRouteReference || !hasPreviousFix) {
    updateRouteReference(
        latitude,
        longitude
    );

    updatePreviousFix(
        latitude,
        longitude,
        timestampMs
    );

    hasRouteReference = true;
    hasPreviousFix = true;

    result.speedKmph = 0.0;
    result.accepted = true;
    result.stationary = true;
    result.moving = false;

    Serial.println("First reliable GPS location accepted.");

    return result;
  }

  /*
   * Distance between two consecutive GPS fixes.
   * Used for speed and stop detection.
   */
  const double fixDistanceMeters =
      TinyGPSPlus::distanceBetween(
          previousFixLatitude,
          previousFixLongitude,
          latitude,
          longitude
      );

  const unsigned long fixElapsedMs =
      timestampMs - previousFixTimestampMs;

  const double positionSpeedKmph =
      calculatePositionSpeedKmph(
          fixDistanceMeters,
          fixElapsedMs
      );

  /*
   * Distance from the last accepted route point.
   * Used to decide when another map point should be sent.
   */
  const double routeDistanceMeters =
      TinyGPSPlus::distanceBetween(
          routeReferenceLatitude,
          routeReferenceLongitude,
          latitude,
          longitude
      );

  result.distanceMeters = routeDistanceMeters;

  /*
   * Every reliable GPS fix becomes the previous fix,
   * even when it is not accepted as a route point.
   */
  updatePreviousFix(
      latitude,
      longitude,
      timestampMs
  );

  Serial.println("----------------------------");

  Serial.print("Fix distance: ");
  Serial.print(fixDistanceMeters, 2);
  Serial.println(" m");

  Serial.print("Route distance: ");
  Serial.print(routeDistanceMeters, 2);
  Serial.println(" m");

  Serial.print("Position speed: ");
  Serial.print(positionSpeedKmph, 2);
  Serial.println(" km/h");

  if (gpsSpeedValid) {
    Serial.print("GPS speed: ");
    Serial.print(gpsSpeedKmph, 2);
    Serial.println(" km/h");
  }

  /*
   * Reject sudden coordinate jumps.
   */
  if (positionSpeedKmph > MAX_POSITION_JUMP_KMPH) {
    movementConfirmationCount = 0;

    Serial.println("GPS jump rejected.");

    return result;
  }

  const double selectedSpeedKmph =
      chooseSpeedKmph(
          gpsSpeedKmph,
          gpsSpeedValid,
          positionSpeedKmph
      );

  Serial.print("Selected speed: ");
  Serial.print(selectedSpeedKmph, 2);
  Serial.println(" km/h");

  // --------------------------------------------------
  // Stationary state
  // --------------------------------------------------

  if (!isMovingState) {
    const bool distanceShowsMovement =
        routeDistanceMeters >=
        START_MOVEMENT_METERS;

    const bool speedShowsMovement =
        selectedSpeedKmph >=
            MIN_MOVING_SPEED_KMPH &&
        selectedSpeedKmph <=
            MAX_VALID_SPEED_KMPH;

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

      Serial.println("State: Stationary");

      return result;
    }

    if (
        movementConfirmationCount <
        START_CONFIRMATION_READINGS
    ) {
      result.speedKmph = 0.0;
      result.stationary = true;
      result.moving = false;

      return result;
    }

    /*
     * Movement has now been confirmed.
     */
    isMovingState = true;

    movementConfirmationCount = 0;
    stopConfirmationCount = 0;

    smoothedSpeedKmph = selectedSpeedKmph;

    result.speedKmph = smoothedSpeedKmph;
    result.accepted = true;
    result.stationary = false;
    result.moving = true;

    updateRouteReference(
        latitude,
        longitude
    );

    Serial.println("Movement state started.");

    return result;
  }

  // --------------------------------------------------
  // Moving state
  // --------------------------------------------------

  const bool lowFixDistance =
      fixDistanceMeters <
      STOP_DISTANCE_METERS;

  const bool lowGpsSpeed =
      !gpsSpeedValid ||
      gpsSpeedKmph <
          STOP_SPEED_KMPH;

  const bool lowPositionSpeed =
      positionSpeedKmph <
      MIN_MOVING_SPEED_KMPH;

  /*
   * Several continuous low-movement readings are
   * required before stopping.
   */
  if (
      lowFixDistance &&
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
     * Send the last accepted route coordinate with
     * speed zero, avoiding a drifting stop point.
     */
    result.latitude = routeReferenceLatitude;
    result.longitude = routeReferenceLongitude;
    result.distanceMeters = 0.0;
    result.speedKmph = 0.0;

    result.accepted = true;
    result.stationary = true;
    result.moving = false;

    Serial.println("Movement stopped. Sending 0.0 km/h.");

    return result;
  }

  /*
   * Do not send another route point until enough
   * route distance has accumulated.
   */
  if (routeDistanceMeters < TRACK_POINT_METERS) {
    result.speedKmph = smoothedSpeedKmph;
    result.stationary = false;
    result.moving = true;

    Serial.println("Moving, waiting for route distance.");

    return result;
  }

  /*
   * Reject invalid moving speeds.
   */
  if (
      selectedSpeedKmph <
          MIN_MOVING_SPEED_KMPH ||
      selectedSpeedKmph >
          MAX_VALID_SPEED_KMPH
  ) {
    result.speedKmph = smoothedSpeedKmph;
    result.stationary = false;
    result.moving = true;

    Serial.println("Route point rejected: invalid speed.");

    return result;
  }

  result.speedKmph =
      smoothSpeed(selectedSpeedKmph);

  result.accepted = true;
  result.stationary = false;
  result.moving = true;

  updateRouteReference(
      latitude,
      longitude
  );

  Serial.print("Route point accepted. Distance: ");
  Serial.print(routeDistanceMeters, 2);
  Serial.print(" m, Speed: ");
  Serial.print(result.speedKmph, 2);
  Serial.println(" km/h");

  return result;
}