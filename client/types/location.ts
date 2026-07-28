export interface Coordinate {
  latitude: number;
  longitude: number;
}

export interface TrackerLocation extends Coordinate {
  speed: number;
  lastUpdated: string;
}
