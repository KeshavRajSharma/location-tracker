#ifndef LOCATION_FILTER_H
#define LOCATION_FILTER_H

struct FilteredLocation {
  double latitude;
  double longitude;
  double speedKmph;
  double distanceMeters;
  bool accepted;
};

void resetLocationFilter();

FilteredLocation filterLocation(
    double latitude,
    double longitude,
    unsigned long timestampMs
);

#endif