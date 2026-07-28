import { useEffect, useRef } from "react";
import { StyleSheet, Text, View } from "react-native";
import MapView, { Marker, Polyline } from "react-native-maps";

import type { Coordinate } from "../types/location";

interface TrackerMapProps {
  coordinates: Coordinate[];
}

export default function TrackerMap({ coordinates }: TrackerMapProps) {
  const mapRef = useRef<MapView>(null);

  const startLocation = coordinates.length > 0 ? coordinates[0] : null;

  const latestLocation =
    coordinates.length > 0 ? coordinates[coordinates.length - 1] : null;

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
    }, 500);

    return () => clearTimeout(timer);
  }, [coordinates]);

  if (!latestLocation) {
    return (
      <View className="h-72 items-center justify-center rounded-3xl border border-slate-200 bg-slate-100">
        <Text className="text-lg font-bold text-slate-700">
          No location available
        </Text>

        <Text className="mt-2 px-6 text-center text-sm text-slate-500">
          GPS coordinates will appear here when tracking starts.
        </Text>
      </View>
    );
  }

  return (
    <View className="h-72 overflow-hidden rounded-3xl border border-slate-200 bg-slate-100">
      <MapView
        ref={mapRef}
        style={StyleSheet.absoluteFillObject}
        initialRegion={{
          latitude: latestLocation.latitude,
          longitude: latestLocation.longitude,
          latitudeDelta: 0.01,
          longitudeDelta: 0.01,
        }}
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

        {startLocation && (
          <Marker
            coordinate={startLocation}
            title="Starting point"
            description="Tracking started here"
            pinColor="green"
          />
        )}

        <Marker
          coordinate={latestLocation}
          title="Current location"
          description="Latest GPS coordinate"
          pinColor="red"
        />
      </MapView>
    </View>
  );
}
