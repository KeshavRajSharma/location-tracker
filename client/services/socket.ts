import type { ApiLocation } from "../types/location";

// Use the same laptop IP used in services/api.ts.
const SERVER_IP = process.env.EXPO_PUBLIC_SERVER_IP;

if (!SERVER_IP) {
  throw new Error("EXPO_PUBLIC_SERVER_IP is not configured");
}

const WEBSOCKET_URL = `ws://${SERVER_IP}:8000/ws/locations`;

interface LocationSocketOptions {
  onLocation: (location: ApiLocation) => void;
  onOpen?: () => void;
  onClose?: () => void;
  onError?: () => void;
}

export function connectLocationSocket({
  onLocation,
  onOpen,
  onClose,
  onError,
}: LocationSocketOptions): WebSocket {
  const socket = new WebSocket(WEBSOCKET_URL);

  socket.onopen = () => {
    console.log("Location WebSocket connected");
    onOpen?.();
  };

  socket.onmessage = (event) => {
    try {
      const location = JSON.parse(event.data) as ApiLocation;
      onLocation(location);
    } catch (error) {
      console.error("Invalid WebSocket location data:", error);
    }
  };

  socket.onerror = () => {
    console.error("Location WebSocket error");
    onError?.();
  };

  socket.onclose = () => {
    console.log("Location WebSocket disconnected");
    onClose?.();
  };

  return socket;
}
