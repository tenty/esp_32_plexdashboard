#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Wi-Fi Credentials
// ============================================================================
#define WIFI_SSID     "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// ============================================================================
// Plex Media Server Configuration
// ============================================================================
// Your Plex Server IP address or Hostname (e.g. "192.168.1.100" or "plex.local")
#define PLEX_SERVER_HOST "192.168.10.92"

// Plex Server Port (Default Plex port is 32400)
#define PLEX_SERVER_PORT 32400

// Plex Authentication Token (X-Plex-Token)
// Can be left empty ("") if unauthenticated local network access is allowed in Plex Settings
#define PLEX_TOKEN       ""

// Use HTTPS (Set true for https://, false for http://)
#define PLEX_USE_HTTPS   false

// Refresh Interval in seconds (e.g., 10 seconds for near real-time updates)
#define API_POLL_INTERVAL_SEC 10

// Default Display Brightness (0 - 255)
#define DEFAULT_BRIGHTNESS 200

#endif // CONFIG_H
