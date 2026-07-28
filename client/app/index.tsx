import { useEffect, useState } from "react";
import { Alert, Pressable, ScrollView, Text, View } from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

import InfoCard from "../components/InfoCard";
import StatusBadge from "../components/StatusBadge";
import TrackerMap from "../components/TrackerMap";
import { mockLocation } from "../data/mockLocation";
import { mockRoute } from "../data/mockRoute";
import type { Coordinate } from "../types/location";

export default function HomeScreen() {
  const [routeCoordinates, setRouteCoordinates] = useState<Coordinate[]>([]);
  const [currentPointIndex, setCurrentPointIndex] = useState(0);
  const [isTracking, setIsTracking] = useState(false);
  const [lastUpdated, setLastUpdated] = useState("Not started");

  const latestCoordinate =
    routeCoordinates.length > 0
      ? routeCoordinates[routeCoordinates.length - 1]
      : null;

  useEffect(() => {
    if (!isTracking) {
      return;
    }

    if (currentPointIndex >= mockRoute.length) {
      setIsTracking(false);
      return;
    }

    const timer = setTimeout(() => {
      const nextCoordinate = mockRoute[currentPointIndex];

      setRouteCoordinates((previousCoordinates) => [
        ...previousCoordinates,
        nextCoordinate,
      ]);

      setCurrentPointIndex((previousIndex) => previousIndex + 1);

      setLastUpdated(
        new Date().toLocaleTimeString([], {
          hour: "2-digit",
          minute: "2-digit",
          second: "2-digit",
        }),
      );
    }, 2000);

    return () => clearTimeout(timer);
  }, [isTracking, currentPointIndex]);

  const handleStartTracking = () => {
    if (currentPointIndex >= mockRoute.length) {
      Alert.alert(
        "Route completed",
        "Clear or refresh the route before starting again.",
      );
      return;
    }

    setIsTracking(true);
  };

  const handleStopTracking = () => {
    setIsTracking(false);
  };

  const handleRefresh = () => {
    setIsTracking(false);
    setRouteCoordinates([]);
    setCurrentPointIndex(0);
    setLastUpdated("Not started");

    Alert.alert("Route reset", "The mock route is ready to track again.");
  };

  const handleClearPath = () => {
    if (isTracking) {
      Alert.alert(
        "Stop tracking first",
        "Please stop tracking before clearing the path.",
      );
      return;
    }

    if (routeCoordinates.length === 0) {
      Alert.alert("No path", "There is no travelled path to clear.");
      return;
    }

    Alert.alert(
      "Clear travelled path",
      "Are you sure you want to clear the current route?",
      [
        {
          text: "Cancel",
          style: "cancel",
        },
        {
          text: "Clear",
          style: "destructive",
          onPress: () => {
            setRouteCoordinates([]);
            setCurrentPointIndex(0);
            setLastUpdated("Not started");
          },
        },
      ],
    );
  };

  return (
    <SafeAreaView
      edges={["top", "left", "right"]}
      className="flex-1 bg-slate-50"
    >
      <ScrollView
        className="flex-1"
        contentContainerStyle={{
          paddingHorizontal: 20,
          paddingTop: 16,
          paddingBottom: 40,
        }}
        showsVerticalScrollIndicator={false}
      >
        <View>
          <Text className="text-3xl font-bold text-slate-900">
            Live Location Tracker
          </Text>

          <Text className="mt-1 text-base text-slate-500">
            ESP32 GPS Monitoring
          </Text>
        </View>

        <View className="mt-5 flex-row flex-wrap gap-3">
          <StatusBadge
            label={isTracking ? "Tracking" : "Tracking Stopped"}
            active={isTracking}
          />

          <StatusBadge label="GPS Fixed" active={mockLocation.gpsFixed} />
        </View>

        <View className="mt-6">
          <TrackerMap coordinates={routeCoordinates} />
        </View>

        <Text className="mb-3 mt-7 text-xl font-bold text-slate-900">
          Current GPS Data
        </Text>

        <View className="flex-row flex-wrap justify-between gap-y-4">
          <InfoCard
            label="Latitude"
            value={
              latestCoordinate ? latestCoordinate.latitude.toFixed(6) : "--"
            }
          />

          <InfoCard
            label="Longitude"
            value={
              latestCoordinate ? latestCoordinate.longitude.toFixed(6) : "--"
            }
          />

          <InfoCard
            label="Speed"
            value={
              isTracking ? `${mockLocation.speed.toFixed(1)} km/h` : "0.0 km/h"
            }
          />

          <InfoCard label="Last Updated" value={lastUpdated} />
        </View>

        <View className="mt-7">
          {!isTracking ? (
            <Pressable
              onPress={handleStartTracking}
              className="items-center justify-center rounded-2xl bg-emerald-600 px-4 py-4 active:bg-emerald-700"
            >
              <Text className="font-bold text-white">Start Tracking</Text>
            </Pressable>
          ) : (
            <Pressable
              onPress={handleStopTracking}
              className="items-center justify-center rounded-2xl bg-orange-500 px-4 py-4 active:bg-orange-600"
            >
              <Text className="font-bold text-white">Stop Tracking</Text>
            </Pressable>
          )}
        </View>

        <View className="mt-3 flex-row gap-3">
          <Pressable
            onPress={handleRefresh}
            className="flex-1 items-center justify-center rounded-2xl bg-blue-600 px-4 py-4 active:bg-blue-700"
          >
            <Text className="font-bold text-white">Reset Route</Text>
          </Pressable>

          <Pressable
            onPress={handleClearPath}
            disabled={routeCoordinates.length === 0}
            className={`flex-1 items-center justify-center rounded-2xl border px-4 py-4 ${
              routeCoordinates.length === 0
                ? "border-slate-200 bg-slate-100"
                : "border-red-200 bg-red-50 active:bg-red-100"
            }`}
          >
            <Text
              className={`font-bold ${
                routeCoordinates.length === 0
                  ? "text-slate-400"
                  : "text-red-600"
              }`}
            >
              Clear Path
            </Text>
          </Pressable>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}
