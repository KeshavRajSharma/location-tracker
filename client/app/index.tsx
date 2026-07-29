import { useCallback, useEffect, useRef, useState } from "react";
import {
  Alert,
  Pressable,
  RefreshControl,
  ScrollView,
  Text,
  View,
} from "react-native";
import { SafeAreaView } from "react-native-safe-area-context";

import InfoCard from "../components/InfoCard";
import StatusBadge from "../components/StatusBadge";
import TrackerMap from "../components/TrackerMap";
import {
  checkBackendHealth,
  clearLocations,
  getLocations,
} from "../services/api";
import { connectLocationSocket } from "../services/socket";
import type { ApiLocation, Coordinate } from "../types/location";

export default function HomeScreen() {
  const [locations, setLocations] = useState<ApiLocation[]>([]);
  const [isTracking, setIsTracking] = useState(false);
  const [isBackendOnline, setIsBackendOnline] = useState(false);
  const [isSocketConnected, setIsSocketConnected] = useState(false);
  const [isRefreshing, setIsRefreshing] = useState(false);
  const [errorMessage, setErrorMessage] = useState<string | null>(null);

  const socketRef = useRef<WebSocket | null>(null);

  const latestLocation =
    locations.length > 0 ? locations[locations.length - 1] : null;

  const routeCoordinates: Coordinate[] = locations.map(
    ({ latitude, longitude }) => ({
      latitude,
      longitude,
    }),
  );

  const loadLocations = useCallback(async () => {
    try {
      const backendOnline = await checkBackendHealth();
      setIsBackendOnline(backendOnline);

      if (!backendOnline) {
        setErrorMessage("Backend is currently unreachable.");
        return;
      }

      const locationHistory = await getLocations();

      setLocations(locationHistory);
      setErrorMessage(null);
    } catch (error) {
      setIsBackendOnline(false);
      setErrorMessage(
        error instanceof Error
          ? error.message
          : "Unable to load location data.",
      );
    }
  }, []);

  useEffect(() => {
    loadLocations();
  }, [loadLocations]);

  useEffect(() => {
    if (!isTracking) {
      socketRef.current?.close();
      socketRef.current = null;
      setIsSocketConnected(false);
      return;
    }

    socketRef.current = connectLocationSocket({
      onLocation: (newLocation) => {
        setLocations((previousLocations) => {
          const alreadyExists = previousLocations.some(
            (location) => location.id === newLocation.id,
          );

          if (alreadyExists) {
            return previousLocations;
          }

          return [...previousLocations, newLocation];
        });

        setIsBackendOnline(true);
        setErrorMessage(null);
      },

      onOpen: () => {
        setIsBackendOnline(true);
        setIsSocketConnected(true);
        setErrorMessage(null);
      },

      onClose: () => {
        socketRef.current = null;
        setIsSocketConnected(false);
      },

      onError: () => {
        setIsSocketConnected(false);
        setErrorMessage("Live connection was interrupted.");
      },
    });

    return () => {
      socketRef.current?.close();
      socketRef.current = null;
      setIsSocketConnected(false);
    };
  }, [isTracking]);

  const handleStartTracking = async () => {
    await loadLocations();
    setIsTracking(true);
  };

  const handleStopTracking = () => {
    setIsTracking(false);
  };

  const handleRefresh = async () => {
    setIsRefreshing(true);

    try {
      await loadLocations();
    } finally {
      setIsRefreshing(false);
    }
  };

  const handleClearPath = () => {
    if (isTracking) {
      Alert.alert(
        "Stop tracking first",
        "Please stop tracking before clearing the path.",
      );
      return;
    }

    if (locations.length === 0) {
      Alert.alert("No path", "There is no path to clear.");
      return;
    }

    Alert.alert(
      "Clear travelled path",
      "This will permanently delete all stored location points.",
      [
        {
          text: "Cancel",
          style: "cancel",
        },
        {
          text: "Clear",
          style: "destructive",
          onPress: async () => {
            try {
              await clearLocations();
              setLocations([]);
              setErrorMessage(null);
            } catch {
              Alert.alert(
                "Clear failed",
                "The stored path could not be cleared.",
              );
            }
          },
        },
      ],
    );
  };

  const formattedTime = latestLocation
    ? new Date(latestLocation.recorded_at).toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
        second: "2-digit",
      })
    : "--";

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
        refreshControl={
          <RefreshControl refreshing={isRefreshing} onRefresh={handleRefresh} />
        }
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
            label={isBackendOnline ? "Backend Online" : "Backend Offline"}
            active={isBackendOnline}
          />

          <StatusBadge
            label={
              isTracking
                ? isSocketConnected
                  ? "Live Tracking"
                  : "Connecting"
                : "Tracking Stopped"
            }
            active={isTracking && isSocketConnected}
          />
        </View>

        {errorMessage && (
          <View className="mt-4 rounded-2xl border border-red-200 bg-red-50 p-4">
            <Text className="font-medium text-red-700">{errorMessage}</Text>
          </View>
        )}

        <View className="mt-6">
          <TrackerMap coordinates={routeCoordinates} />
        </View>

        <Text className="mb-3 mt-7 text-xl font-bold text-slate-900">
          Current GPS Data
        </Text>

        <View className="flex-row flex-wrap justify-between gap-y-4">
          <InfoCard
            label="Latitude"
            value={latestLocation ? latestLocation.latitude.toFixed(6) : "--"}
          />

          <InfoCard
            label="Longitude"
            value={latestLocation ? latestLocation.longitude.toFixed(6) : "--"}
          />

          <InfoCard
            label="Speed"
            value={
              latestLocation
                ? `${latestLocation.speed.toFixed(1)} km/h`
                : "0.0 km/h"
            }
          />

          <InfoCard label="Last Updated" value={formattedTime} />
        </View>

        <View className="mt-7">
          {!isTracking ? (
            <Pressable
              onPress={handleStartTracking}
              disabled={!isBackendOnline}
              className={`items-center rounded-2xl px-4 py-4 ${
                isBackendOnline
                  ? "bg-emerald-600 active:bg-emerald-700"
                  : "bg-slate-300"
              }`}
            >
              <Text
                className={`font-bold ${
                  isBackendOnline ? "text-white" : "text-slate-500"
                }`}
              >
                Start Tracking
              </Text>
            </Pressable>
          ) : (
            <Pressable
              onPress={handleStopTracking}
              className="items-center rounded-2xl bg-orange-500 px-4 py-4 active:bg-orange-600"
            >
              <Text className="font-bold text-white">Stop Tracking</Text>
            </Pressable>
          )}
        </View>

        <View className="mt-3 flex-row gap-3">
          <Pressable
            onPress={handleRefresh}
            className="flex-1 items-center rounded-2xl bg-blue-600 px-4 py-4 active:bg-blue-700"
          >
            <Text className="font-bold text-white">Refresh</Text>
          </Pressable>

          <Pressable
            onPress={handleClearPath}
            disabled={locations.length === 0 || isTracking}
            className={`flex-1 items-center rounded-2xl border px-4 py-4 ${
              locations.length === 0 || isTracking
                ? "border-slate-200 bg-slate-100"
                : "border-red-200 bg-red-50 active:bg-red-100"
            }`}
          >
            <Text
              className={`font-bold ${
                locations.length === 0 || isTracking
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
