import type { ApiLocation } from "../types/location";

// Replace this with your laptop's actual local Wi-Fi IP.
const SERVER_IP = process.env.EXPO_PUBLIC_SERVER_IP;

if (!SERVER_IP) {
  throw new Error("EXPO_PUBLIC_SERVER_IP is not configured");
}

const API_BASE_URL = `http://${SERVER_IP}:8000`;

async function handleResponse<T>(response: Response): Promise<T> {
  if (!response.ok) {
    const message = `Request failed with status ${response.status}`;
    throw new Error(message);
  }

  return response.json() as Promise<T>;
}

export async function checkBackendHealth(): Promise<boolean> {
  try {
    const response = await fetch(`${API_BASE_URL}/health`);
    return response.ok;
  } catch {
    return false;
  }
}

export async function getLocations(): Promise<ApiLocation[]> {
  const response = await fetch(`${API_BASE_URL}/locations`);
  return handleResponse<ApiLocation[]>(response);
}

export async function getLatestLocation(): Promise<ApiLocation | null> {
  const response = await fetch(`${API_BASE_URL}/locations/latest`);

  if (response.status === 404) {
    return null;
  }

  return handleResponse<ApiLocation>(response);
}

export async function clearLocations(): Promise<void> {
  const response = await fetch(`${API_BASE_URL}/locations`, {
    method: "DELETE",
  });

  if (!response.ok && response.status !== 204) {
    throw new Error(`Unable to clear locations: ${response.status}`);
  }
}
