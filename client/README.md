# Location Tracker — Client

Expo React Native frontend for the live GPS location tracker project.

## Stack

- **Expo SDK** 54 with New Architecture enabled
- **Expo Router** v6 (file-based routing)
- **NativeWind** v4 (Tailwind CSS for React Native)
- **TypeScript** (strict mode)
- **React Native** 0.81

## Project structure

```
client/
├── app/
│   ├── _layout.tsx     # Root layout (Stack navigator)
│   └── index.tsx       # Home screen (map view — coming soon)
├── assets/
│   └── images/         # App icons and splash screen
├── global.css          # Tailwind base imports
├── app.json            # Expo app configuration
├── babel.config.js     # Babel with NativeWind v4 preset
├── metro.config.js     # Metro with NativeWind CSS pipeline
├── tailwind.config.js  # Tailwind/NativeWind content config
├── tsconfig.json       # TypeScript (extends expo/tsconfig.base)
└── package.json
```

## Running locally

```bash
npm install        # install dependencies
npm start          # start Metro bundler (scan QR with Expo Go)
npm run android    # open on Android emulator / device
npm run ios        # open on iOS simulator / device
```
