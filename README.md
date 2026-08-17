# ESP32-C6 Plex Active Streamer Monitor

A real-time desktop dashboard for the **Waveshare ESP32-C6-Touch-LCD-1.47** (172×320 IPS LCD with Capacitive Touch).

It connects directly to your **Plex Media Server** to monitor and display how many people are currently streaming at once, along with user session details.

---

## Features

* 📺 **Real-time Stream Count**: Big, bold display of active Plex streamers.
* 👥 **Stream Details**: Displays active usernames, media titles, and player states (playing/paused).
* 👆 **Capacitive Touch Refresh**: Tap the screen anytime to force an immediate refresh.
* ⏱️ **Auto Polling**: Periodically polls your Plex server (default: every 10 seconds).
* 🎨 **Plex Signature Theme**: High-contrast pitch-black dark theme with Plex Amber (`#E5A00D`) accents.

---

## Hardware Features & Pinout

| Function | Pin (ESP32-C6) | Description |
| :--- | :--- | :--- |
| **LCD MOSI** | GPIO 6 | SPI Data |
| **LCD SCLK** | GPIO 7 | SPI Clock |
| **LCD CS** | GPIO 14 | Chip Select |
| **LCD DC** | GPIO 15 | Data / Command |
| **LCD RST** | GPIO 21 | Hardware Reset |
| **LCD BL** | GPIO 22 | Backlight Control |
| **Touch SDA** | GPIO 18 | I2C Data |
| **Touch SCL** | GPIO 19 | I2C Clock |
| **Touch RST** | GPIO 20 | Touch Controller Reset |

---

## Setup & Configuration

1. Open [`src/config.h`](file:///Users/edwinyeoh/GIT/ESP-C6/src/config.h):
   ```cpp
   #define WIFI_SSID        "Your_WiFi_Network"
   #define WIFI_PASSWORD    "Your_WiFi_Password"

   #define PLEX_SERVER_HOST "192.168.1.100" // Your Plex Server IP or Hostname
   #define PLEX_SERVER_PORT 32400           // Default Plex Port
   #define PLEX_TOKEN       "X-Plex-Token"  // Your Plex Auth Token
   ```

2. **How to find your `X-Plex-Token`**:
   * Log in to **Plex Web App** on your local browser.
   * Navigate to any media item (movie or episode) and click **"..." (More)** $\rightarrow$ **"View XML"**.
   * Look at the browser URL bar for `X-Plex-Token=...` and copy your token value into `PLEX_TOKEN` in `src/config.h`.

---

## How to Build & Flash

### Option A: PlatformIO (Recommended)

```bash
# Build firmware
pio run

# Upload to connected ESP32-C6
pio run --target upload

# Open Serial Monitor
pio device monitor
```

### Option B: Arduino IDE

1. Open **Arduino IDE**.
2. Install **ESP32** board support by Espressif (version **3.0.0+** for ESP32-C6 support).
3. Select Board: **ESP32C6 Dev Module**.
4. Install required libraries from Library Manager:
   * `ArduinoJson` (v7.x)
   * `GFX Library for Arduino` (by Moon On Our Nation)
5. Open `src/main.cpp` (or rename to `.ino`), update `src/config.h`, and click **Upload**!
