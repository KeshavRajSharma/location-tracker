# Location Tracker

A live GPS location tracker using an ESP32 + NEO-6M GPS module, a FastAPI backend, and an Expo React Native frontend.

## Project structure

```
location-tracker/
├── client/          # Expo React Native (TypeScript, NativeWind, Expo Router)
├── server/          # FastAPI backend (Python, SQLAlchemy, SQLite)
└── README.md
```

## Planned feature set

- ESP32 reads GPS coordinates from a NEO-6M module and displays them on a small OLED screen
- ESP32 POSTs coordinates to the FastAPI backend over Wi-Fi
- FastAPI stores every coordinate in a SQLite database
- React Native app polls or subscribes to the backend and renders:
  - An OpenStreetMap tile layer
  - A marker for the latest location
  - A solid polyline for the full travelled path

## Quick start

### Backend

```bash
cd server
python -m venv venv
source venv/bin/activate      # Windows: venv\Scripts\activate
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

### Frontend

```bash
cd client
npm install
npm start          # scan QR code with Expo Go
```
