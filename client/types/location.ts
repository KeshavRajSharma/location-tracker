export interface Coordinate {
  latitude: number;
  longitude: number;
}

export interface TrackerLocation extends Coordinate {
  speed: number;
  lastUpdated: string;
}

export interface ApiLocation extends Coordinate {
  id: number;
  device_id: string;
  speed: number;
  recorded_at: string;
}
