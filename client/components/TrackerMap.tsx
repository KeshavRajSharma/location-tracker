import { useEffect, useRef } from "react";
import { StyleSheet, Text, View } from "react-native";
import MapView, { Marker, Polyline, type Region } from "react-native-maps";

import type { Coordinate } from "../types/location";

interface TrackerMapProps {
  coordinates: Coordinate[];
}

/*
 * Default map region shown while the ESP32 has not
 * provided any valid GPS location.
 *
 * This currently shows the Kathmandu area.
 */
const DEFAULT_REGION: Region = {
  latitude: 27.7172,
  longitude: 85.324,
  latitudeDelta: 0.08,
  longitudeDelta: 0.08,
};

export default function TrackerMap({ coordinates }: TrackerMapProps) {
  const mapRef = useRef<MapView>(null);

  const startLocation = coordinates.length > 0 ? coordinates[0] : null;

  const latestLocation =
    coordinates.length > 0 ? coordinates[coordinates.length - 1] : null;

  /*
   * When the first GPS location becomes available,
   * move the map immediately from the default region
   * to the tracker location.
   */
  useEffect(() => {
    if (coordinates.length !== 1 || !latestLocation) {
      return;
    }

    const timer = setTimeout(() => {
      mapRef.current?.animateToRegion(
        {
          latitude: latestLocation.latitude,
          longitude: latestLocation.longitude,
          latitudeDelta: 0.01,
          longitudeDelta: 0.01,
        },
        700,
      );
    }, 200);

    return () => clearTimeout(timer);
  }, [coordinates.length, latestLocation]);

  /*
   * When a route contains two or more points,
   * resize the map so the complete route is visible.
   */
  useEffect(() => {
    if (coordinates.length < 2) {
      return;
    }

    const timer = setTimeout(() => {
      mapRef.current?.fitToCoordinates(coordinates, {
        edgePadding: {
          top: 60,
          right: 60,
          bottom: 60,
          left: 60,
        },
        animated: true,
      });
    }, 300);

    return () => clearTimeout(timer);
  }, [coordinates]);

  return (
    <View className="h-72 overflow-hidden rounded-3xl border border-slate-200 bg-slate-100">
      <MapView
        ref={mapRef}
        style={StyleSheet.absoluteFillObject}
        initialRegion={
          latestLocation
            ? {
                latitude: latestLocation.latitude,
                longitude: latestLocation.longitude,
                latitudeDelta: 0.01,
                longitudeDelta: 0.01,
              }
            : DEFAULT_REGION
        }
        loadingEnabled
        rotateEnabled={false}
        pitchEnabled={false}
        toolbarEnabled={false}
        showsCompass={false}
      >
        {coordinates.length > 1 && (
          <Polyline
            coordinates={coordinates}
            strokeWidth={5}
            strokeColor="#2563EB"
            lineCap="round"
            lineJoin="round"
          />
        )}

        {coordinates.length > 1 && startLocation && (
          <Marker
            coordinate={startLocation}
            title="Starting point"
            description="Tracking started here"
            pinColor="green"
          />
        )}

        {latestLocation && (
          <Marker
            coordinate={latestLocation}
            title={
              coordinates.length > 1 ? "Current location" : "Tracker location"
            }
            description="Latest ESP32 GPS coordinate"
            pinColor="red"
          />
        )}
      </MapView>

      {!latestLocation && (
        <View
          pointerEvents="none"
          className="absolute bottom-4 left-4 right-4 rounded-2xl bg-white/95 px-4 py-3 shadow"
        >
          <Text className="text-center font-bold text-slate-800">
            No GPS location available
          </Text>

          <Text className="mt-1 text-center text-sm text-slate-500">
            Waiting for the ESP32 to provide its first valid GPS coordinate.
          </Text>
        </View>
      )}
    </View>
  );
}
