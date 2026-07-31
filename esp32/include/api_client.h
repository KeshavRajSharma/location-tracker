#ifndef API_CLIENT_H
#define API_CLIENT_H

bool checkBackendHealth();

bool sendLocation(
    double latitude,
    double longitude,
    double speedKmph
);

#endif