# Live Location Tracker

A real-time GPS tracking system built with an **ESP32**, **NEO-6M GPS module**, **SSD1306 OLED display**, **FastAPI**, and a **React Native Expo mobile application**.

The ESP32 receives GPS coordinates, filters inaccurate readings, and sends valid location data to the FastAPI backend. The backend stores the data in SQLite and broadcasts new locations to the mobile application using WebSocket communication.

The mobile application displays the latest tracker position and draws the travelled route during an active tracking session.

---

## Project Overview

The Live Location Tracker is designed to track a person carrying an ESP32-based GPS device.

It combines embedded hardware, backend development, database storage, real-time communication, and mobile map visualization in a single system.

### Main Capabilities

- Live GPS location monitoring
- Real-time latitude and longitude updates
- Walking-speed calculation
- Travelled-route visualization
- Start and stop tracking controls
- Path clearing
- Backend connection monitoring
- GPS noise filtering
- OLED hardware-status display
- WebSocket-based live updates

---

## System Architecture

```text
NEO-6M GPS Module
        │
        ▼
      ESP32
        │
        │ HTTP POST
        ▼
 FastAPI Backend
        │
        ├── SQLite Database
        │
        └── WebSocket
                │
                ▼
      React Native Mobile App
```

---

## How the System Works

1. The NEO-6M GPS module receives location information from satellites.
2. The ESP32 reads latitude and longitude values from the GPS module.
3. GPS drift, small coordinate changes, and unrealistic movements are filtered.
4. Valid location data is sent to the FastAPI backend through an HTTP request.
5. The backend stores the location in an SQLite database.
6. The backend broadcasts new location data through WebSocket.
7. The mobile application receives the location and updates the map.
8. During tracking, the application draws the travelled route in real time.

---

## Features

### Mobile Application

- Displays the latest ESP32 tracker location
- Shows only one location marker when the application opens
- Does not display old route history during application startup
- Starts a new tracking session when **Start Tracking** is pressed
- Uses the latest GPS position as the starting point
- Receives new location data through WebSocket
- Draws the current tracking-session route
- Keeps the completed route visible after tracking is stopped
- Removes the route only after **Clear Path** is pressed
- Preserves one latest-location marker after clearing the route
- Displays speed only during active tracking
- Shows `--` before tracking starts and after tracking stops
- Displays a general map when GPS data is unavailable
- Automatically focuses on the first valid GPS location
- Shows backend and live-tracking connection status
- Supports manual refresh

### ESP32 Tracker

- Connects to Wi-Fi
- Checks backend availability
- Reads GPS data
- Calculates movement speed
- Filters small GPS coordinate changes
- Rejects unrealistic movement
- Requires consecutive movement readings
- Sends accepted data to the backend
- Displays system information on the OLED screen
- Automatically reconnects to Wi-Fi when disconnected

### Backend

- FastAPI REST API
- SQLite database
- SQLAlchemy ORM
- WebSocket live updates
- Location history storage
- Latest-location retrieval
- Location deletion
- Backend health checking
- Automatic API documentation

---

## Hardware Components

- ESP32 development board
- NEO-6M GPS module
- SSD1306 OLED display
- Breadboard
- Jumper wires
- USB cable or portable power source
- Smartphone for running the mobile application
- Laptop for running the backend server

---

<!-- ## Hardware Setup

<p align="center">
  <img src="assets/hardware-setup.jpg" alt="ESP32 GPS Tracker Hardware Setup" width="700">
</p> -->

The hardware consists of an ESP32 development board connected to a NEO-6M GPS module and an SSD1306 OLED display.

Place your hardware image in:

```text
assets/hardware-setup.jpg
```

The project should contain:

```text
location-tracker/
├── assets/
│   └── hardware-setup.jpg
├── client/
├── server/
├── esp32/
└── README.md
```

---

## Hardware Connections

### NEO-6M GPS Module to ESP32

| GPS Module | ESP32      |
| ---------- | ---------- |
| VCC        | 3.3V or 5V |
| GND        | GND        |
| TX         | GPIO 16    |
| RX         | GPIO 17    |

The GPS module sends data through its `TX` pin to the ESP32 `RX` pin.

---

### SSD1306 OLED Display to ESP32

| OLED Display | ESP32   |
| ------------ | ------- |
| VCC          | 3.3V    |
| GND          | GND     |
| SDA          | GPIO 21 |
| SCL          | GPIO 22 |

The OLED I2C address used by the project is:

```text
0x3C
```

---

## Technology Stack

### Mobile Application

- React Native
- Expo
- Expo Router
- TypeScript
- NativeWind
- React Native Maps
- WebSocket

### Backend

- Python
- FastAPI
- SQLAlchemy
- SQLite
- Uvicorn
- WebSocket

### Embedded System

- ESP32
- Arduino framework
- PlatformIO
- TinyGPSPlus
- ArduinoJson
- Adafruit GFX
- Adafruit SSD1306

---

## Project Structure

```text
location-tracker/
│
├── assets/
│   └── hardware-setup.jpg
│
├── client/
│   ├── app/
│   │   ├── _layout.tsx
│   │   └── index.tsx
│   │
│   ├── components/
│   │   ├── InfoCard.tsx
│   │   ├── StatusBadge.tsx
│   │   └── TrackerMap.tsx
│   │
│   ├── services/
│   │   ├── api.ts
│   │   └── socket.ts
│   │
│   ├── types/
│   │   └── location.ts
│   │
│   └── package.json
│
├── server/
│   ├── app/
│   │   ├── routes/
│   │   │   ├── __init__.py
│   │   │   └── locations.py
│   │   │
│   │   ├── __init__.py
│   │   ├── database.py
│   │   ├── main.py
│   │   ├── models.py
│   │   ├── schemas.py
│   │   └── websocket_manager.py
│   │
│   └── requirements.txt
│
├── esp32/
│   ├── include/
│   │   ├── api_client.h
│   │   ├── config.example.h
│   │   └── location_filter.h
│   │
│   ├── src/
│   │   ├── api_client.cpp
│   │   ├── location_filter.cpp
│   │   └── main.cpp
│   │
│   └── platformio.ini
│
└── README.md
```

---

## Backend Setup

Open a terminal in the project root and enter the backend folder:

```bash
cd server
```

Create a Python virtual environment:

```bash
python3 -m venv venv
```

Activate it on macOS or Linux:

```bash
source venv/bin/activate
```

Activate it on Windows:

```bash
venv\Scripts\activate
```

Install the required Python packages:

```bash
pip install -r requirements.txt
```

Start the FastAPI backend:

```bash
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

The backend will be available at:

```text
http://YOUR_LAPTOP_IP:8000
```

Interactive API documentation will be available at:

```text
http://YOUR_LAPTOP_IP:8000/docs
```

---

## Mobile Application Setup

Open another terminal and enter the client folder:

```bash
cd client
```

Install the required packages:

```bash
npm install
```

Create a `.env` file inside the `client` folder:

```env
EXPO_PUBLIC_SERVER_IP=YOUR_LAPTOP_IP
```

Example:

```env
EXPO_PUBLIC_SERVER_IP=192.168.201.250
```

Start the Expo application:

```bash
npx expo start --clear
```

Open the application on a physical Android or iOS device using Expo Go.

The smartphone and laptop must be connected to the same local Wi-Fi network or mobile hotspot.

---

## ESP32 Setup

Open the `esp32` folder as a PlatformIO project.

Create the following configuration file:

```text
esp32/include/config.h
```

Add your Wi-Fi and backend information:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define SERVER_IP "YOUR_LAPTOP_IP"
#define SERVER_PORT 8000

#define DEVICE_ID "tracker_01"

#endif
```

Example server configuration:

```cpp
#define SERVER_IP "192.168.201.250"
#define SERVER_PORT 8000
```

Build and upload the firmware using PlatformIO.

After uploading, open the PlatformIO Serial Monitor using:

```text
115200 baud
```

---

## API Endpoints

| Method    | Endpoint            | Description                     |
| --------- | ------------------- | ------------------------------- |
| GET       | `/health`           | Check backend availability      |
| POST      | `/locations`        | Save a new GPS location         |
| GET       | `/locations/latest` | Retrieve the latest location    |
| GET       | `/locations`        | Retrieve saved location history |
| DELETE    | `/locations`        | Delete stored locations         |
| WebSocket | `/ws/locations`     | Receive live location updates   |

---

## Location Data Format

The ESP32 sends GPS data to the backend in JSON format:

```json
{
  "device_id": "tracker_01",
  "latitude": 27.7172,
  "longitude": 85.324,
  "speed": 2.4
}
```

### Fields

| Field       | Description                              |
| ----------- | ---------------------------------------- |
| `device_id` | Identifier assigned to the ESP32 tracker |
| `latitude`  | Current GPS latitude                     |
| `longitude` | Current GPS longitude                    |
| `speed`     | Filtered speed in kilometres per hour    |

---

## GPS Speed Calculation

The tracker calculates speed using the distance between two accepted GPS positions.

Both latitude and longitude changes are used.

```text
Speed = Distance travelled ÷ Elapsed time
```

The calculated speed is initially measured in metres per second and then converted to kilometres per hour:

```text
Speed in km/h = Speed in m/s × 3.6
```

The project uses the following TinyGPSPlus function to calculate geographic distance:

```cpp
TinyGPSPlus::distanceBetween(...)
```

This function calculates the distance between the previous latitude-longitude pair and the latest latitude-longitude pair.

---

## GPS Filtering

GPS modules can report small coordinate changes even when the device is stationary. This behaviour is known as GPS drift.

The project applies movement filtering to reduce false movement and unstable speed values.

```cpp
constexpr double MIN_MOVEMENT_METERS = 2.5;
constexpr double MAX_WALKING_SPEED_KMPH = 3.0;
constexpr double SPEED_SMOOTHING_FACTOR = 0.20;
constexpr int REQUIRED_MOVEMENT_READINGS = 3;
```

### Filtering Process

- Movement below `2.5 metres` is treated as GPS noise
- Speed above `3.0 km/h` is rejected
- Three consecutive valid movement readings are required
- Sudden speed changes are smoothed
- Invalid coordinates are rejected
- Only accepted points are sent to the backend

These values are configured for a small classroom walking demonstration.

> A maximum speed of `3.0 km/h` may reject faster normal walking. It can be increased when testing in a larger outdoor area.

---

## GPS Accuracy Considerations

GPS accuracy can be affected by:

- Indoor environments
- Buildings and walls
- Low satellite count
- Weak satellite signals
- Reflected satellite signals
- Nearby electrical interference
- Poor antenna position

For better tracking results:

- Use the tracker outdoors
- Keep the GPS antenna facing upward
- Avoid covering the GPS antenna
- Wait for at least five or six satellites
- Allow the module time to obtain a stable GPS fix

---

## Application Behaviour

### When the Application Opens

- The application checks the backend connection
- The latest saved ESP32 location is requested
- Only one latest-location marker is displayed
- Old route history is not drawn
- Tracking status shows **Tracking Stopped**
- Speed displays `--`

---

### When GPS Data Is Unavailable

- A general map is still displayed
- No location marker is shown
- The map displays a message that GPS data is unavailable
- Latitude, longitude, speed, and time display `--`
- The application waits for the first valid GPS coordinate

When the first valid coordinate becomes available:

- The unavailable-location message disappears
- The map automatically focuses on the tracker location
- One tracker marker is displayed

---

### When Start Tracking Is Pressed

- The latest tracker position becomes the starting point
- A new tracking session begins
- The WebSocket connection opens
- New accepted GPS points are added to the route
- A green marker displays the starting point
- A red marker displays the latest point
- A blue line displays the travelled route
- Speed becomes visible

---

### When Stop Tracking Is Pressed

- The live WebSocket connection closes
- New points are no longer added to the current route
- The completed route remains visible
- The final tracker position remains visible
- Speed returns to `--`
- The route remains until **Clear Path** is pressed

---

### When Tracking Is Started Again

- The previous session route is replaced
- A fresh route begins from the latest available GPS position
- Only new points from the new tracking session are displayed

---

### When Clear Path Is Pressed

- Tracking must first be stopped
- The completed route is removed
- The latest known position remains visible as one marker
- No blue route is displayed
- Speed remains `--`
- The map returns to its initial single-marker state

---

## Map Markers

| Map Element  | Meaning                                        |
| ------------ | ---------------------------------------------- |
| Green marker | Starting point of the current tracking session |
| Red marker   | Latest tracker location                        |
| Blue line    | Travelled route                                |
| No marker    | GPS location is not yet available              |

---

## OLED Display

The OLED screen provides hardware status without requiring the mobile application.

It can display:

```text
Wi-Fi connection status
Backend connection status
Latitude
Longitude
Filtered speed
Satellite count
GPS location status
```

Example:

```text
WiFi: Connected
Backend: Online
Lat: 27.617827
Lon: 85.536897
Speed: 1.9 km/h
Satellites: 6
```

---

## Running the Complete System

### 1. Start the Backend

```bash
cd server
source venv/bin/activate
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

### 2. Start the Mobile Application

```bash
cd client
npx expo start --clear
```

### 3. Power the ESP32

Connect the ESP32 to a USB power source or portable battery.

### 4. Wait for Connections

Confirm that:

- The ESP32 is connected to Wi-Fi
- The OLED displays **Backend Online**
- The GPS module has obtained satellite signals
- The mobile application displays **Backend Online**

### 5. Begin Tracking

1. Open the mobile application.
2. Wait for the initial tracker marker.
3. Press **Start Tracking**.
4. Walk while carrying the ESP32 tracker.
5. Observe the route update on the map.
6. Press **Stop Tracking** after completing the route.
7. Press **Clear Path** when the route is no longer needed.

---

## Troubleshooting

### Backend Shows Offline

Check that:

- The FastAPI server is running
- The laptop and phone are connected to the same network
- The configured laptop IP address is correct
- The backend is running on port `8000`
- The phone can access the backend health endpoint

Test the backend from the phone browser:

```text
http://YOUR_LAPTOP_IP:8000/health
```

---

### ESP32 Cannot Reach the Backend

Check that:

- `SERVER_IP` contains the laptop IP address
- The ESP32 and laptop are connected to the same network
- The FastAPI server uses `--host 0.0.0.0`
- The backend port is correct
- The ESP32 was uploaded again after changing its configuration

---

### GPS Location Is Not Available

Check that:

- The GPS module wiring is correct
- GPS `TX` is connected to ESP32 GPIO `16`
- The GPS antenna has a clear view of the sky
- The device is outdoors or near an open window
- The GPS module has obtained enough satellites

---

### Map Does Not Show a Route

Check that:

- **Start Tracking** has been pressed
- The live-tracking status is connected
- The ESP32 is sending accepted GPS points
- Movement is greater than the configured minimum distance
- The backend is receiving `POST /locations` requests

---

### Speed Remains Zero

Possible reasons include:

- Movement is below `2.5 metres`
- Three consecutive readings have not yet been confirmed
- Walking speed exceeds the configured `3.0 km/h` limit
- GPS readings are being rejected as inaccurate
- The elapsed movement interval is too short

---

### Speed Appears While Stationary

Possible causes include:

- GPS coordinate drift
- Weak satellite signals
- Indoor testing
- Reflected GPS signals
- A coordinate jump greater than the minimum movement threshold

The consecutive-reading filter reduces this behaviour, but low-cost GPS modules may still occasionally report inaccurate movement.

---

### Route Does Not Appear Immediately

The filter requires three consecutive valid movement readings.

Therefore, the first few coordinate changes may be ignored before the route begins updating.

---

## Future Improvements

- Multiple tracker support
- User authentication
- Tracker-device selection
- Route history by date
- Total distance calculation
- Average-speed calculation
- Battery-level monitoring
- Geofencing alerts
- Emergency notification button
- Cloud backend deployment
- Background mobile tracking
- Offline map support
- Route export
- Kalman-filter GPS processing
- Satellite-count storage
- Tracker connection status
- Session-based database storage

---

## Project Purpose

This project was developed as an academic prototype to demonstrate the integration of:

- Embedded-system development
- GPS data processing
- REST API communication
- WebSocket communication
- Mobile application development
- Database management
- Real-time map visualization
- Hardware and software integration

The system demonstrates how an IoT GPS device can communicate with a backend server and provide live location updates to a mobile application.
