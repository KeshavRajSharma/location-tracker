export interface LocationData {
  latitude: number;
  longitude: number;
  speed: number;
  lastUpdated: string;
  deviceOnline: boolean;
  gpsFixed: boolean;
}

export const mockLocation: LocationData = {
  latitude: 27.7172,
  longitude: 85.324,
  speed: 5.4,
  lastUpdated: "Just now",
  deviceOnline: true,
  gpsFixed: true,
};
