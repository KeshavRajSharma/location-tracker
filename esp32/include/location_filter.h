#ifndef LOCATION_FILTER_H
#define LOCATION_FILTER_H

struct FilteredLocation {
  double latitude;
  double longitude;
  double speedKmph;
  double distanceMeters;

  bool accepted;
  bool stationary;
  bool moving;
  bool qualityAccepted;
};

void resetLocationFilter();

FilteredLocation filterLocation(
    double latitude,
    double longitude,
    double gpsSpeedKmph,
    bool gpsSpeedValid,
    unsigned int satellites,
    double hdop,
    bool hdopValid,
    unsigned long timestampMs
);

#endif