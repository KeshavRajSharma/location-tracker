import { useState } from "react";
import { Alert, Pressable, ScrollView, Text, View } from "react-native";

import { SafeAreaView } from "react-native-safe-area-context";

import InfoCard from "../components/InfoCard";
import StatusBadge from "../components/StatusBadge";
import { mockLocation } from "../data/mockLocation";

export default function HomeScreen() {
  const [lastUpdated, setLastUpdated] = useState(mockLocation.lastUpdated);

  const handleRefresh = () => {
    const currentTime = new Date().toLocaleTimeString([], {
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
    });

    setLastUpdated(currentTime);
  };

  const handleClearPath = () => {
    Alert.alert(
      "Clear Path",
      "No travelled path is available yet. The map will be added in the next phase.",
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
            label={mockLocation.deviceOnline ? "Online" : "Offline"}
            active={mockLocation.deviceOnline}
          />

          <StatusBadge
            label={mockLocation.gpsFixed ? "GPS Fixed" : "No GPS Fix"}
            active={mockLocation.gpsFixed}
          />
        </View>

        <View className="mt-6 h-72 items-center justify-center rounded-3xl border border-slate-200 bg-slate-200">
          <View className="h-16 w-16 items-center justify-center rounded-full bg-white">
            <Text className="text-3xl">📍</Text>
          </View>

          <Text className="mt-4 text-lg font-bold text-slate-700">
            Map will appear here
          </Text>

          <Text className="mt-1 px-6 text-center text-sm text-slate-500">
            The live location and travelled path will be shown in this section.
          </Text>
        </View>

        <Text className="mb-3 mt-7 text-xl font-bold text-slate-900">
          Current GPS Data
        </Text>

        <View className="flex-row flex-wrap justify-between gap-y-4">
          <InfoCard label="Latitude" value={mockLocation.latitude.toFixed(4)} />

          <InfoCard
            label="Longitude"
            value={mockLocation.longitude.toFixed(4)}
          />

          <InfoCard
            label="Speed"
            value={`${mockLocation.speed.toFixed(1)} km/h`}
          />

          <InfoCard label="Last Updated" value={lastUpdated} />
        </View>

        <View className="mt-7 flex-row gap-3">
          <Pressable
            onPress={handleRefresh}
            className="flex-1 items-center justify-center rounded-2xl bg-blue-600 px-4 py-4 active:bg-blue-700"
          >
            <Text className="font-bold text-white">Refresh</Text>
          </Pressable>

          <Pressable
            onPress={handleClearPath}
            className="flex-1 items-center justify-center rounded-2xl border border-red-200 bg-red-50 px-4 py-4 active:bg-red-100"
          >
            <Text className="font-bold text-red-600">Clear Path</Text>
          </Pressable>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}
