import type { TrackerLocation } from "../types/location";

export interface LocationData extends TrackerLocation {
  deviceOnline: boolean;
  gpsFixed: boolean;
}

export const mockLocation: LocationData = {
  latitude: 27.7195,
  longitude: 85.3268,
  speed: 5.4,
  lastUpdated: "Just now",
  deviceOnline: true,
  gpsFixed: true,
};
