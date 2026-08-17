#include <Arduino.h>
#include <WiFi.h>
#include <Arduino_GFX_Library.h>

#include "config.h"
#include "touch_driver.h"
#include "plex_client.h"
#include "ui_dashboard.h"

// ----------------------------------------------------------------------------
// ST7789 / JD9853 LCD Setup for Waveshare 1.47" (172x320)
// ----------------------------------------------------------------------------
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    PIN_LCD_DC,   // DC
    PIN_LCD_CS,   // CS
    PIN_LCD_SCLK, // SCK
    PIN_LCD_MOSI, // MOSI
    GFX_NOT_DEFINED, // MISO
    FSPI          // SPI host
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus,
    PIN_LCD_RST,  // RST
    0,            // Rotation 0
    false,        // IPS set to false (fixes inverted color RAM where 0x0000 was showing up bright/inverted)
    172,          // Width
    320,          // Height
    34,           // col offset 1
    0,            // row offset 1
    34,           // col offset 2
    0             // row offset 2
);

UIDashboard dashboard(gfx);
TouchDriver touchDriver;

PlexMetrics currentMetrics;

unsigned long lastApiFetchTime = 0;
bool wifiStatus = false;

// ----------------------------------------------------------------------------
// Helper: Connect Wi-Fi
// ----------------------------------------------------------------------------
void setupWiFi() {
  Serial.print("[WIFI] Connecting to SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiStatus = true;
    Serial.println("\n[WIFI] Connected! IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiStatus = false;
    Serial.println("\n[WIFI] Connection Failed (Running in Offline mode)");
  }
}

// ----------------------------------------------------------------------------
// Helper: Fetch Plex Stream Metrics & Refresh UI
// ----------------------------------------------------------------------------
void refreshPlexMetrics() {
  Serial.println("[PLEX] Fetching active streaming sessions from Plex Server...");
  
  currentMetrics = PlexClient::fetchMetrics(
      PLEX_SERVER_HOST,
      PLEX_SERVER_PORT,
      PLEX_TOKEN,
      PLEX_USE_HTTPS
  );

  Serial.printf("[PLEX] Server Online: %s | Active Streamers: %d\n",
                currentMetrics.serverOnline ? "YES" : "NO",
                currentMetrics.activeStreamers);

  dashboard.renderHeader(currentMetrics.serverName, wifiStatus, currentMetrics.serverOnline);
  dashboard.renderMainStreamerCard(currentMetrics);
  dashboard.renderSessionDetailsCard(currentMetrics);
  dashboard.renderFooterBar("Sync: " + String(millis() / 1000) + "s ago");
  
  lastApiFetchTime = millis();
}

// ----------------------------------------------------------------------------
// Arduino Setup
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== Waveshare ESP32-C6 Plex Streamer Dashboard Starting ===");

  // Backlight PWM initialization (Set to ~75% brightness)
  pinMode(PIN_LCD_BL, OUTPUT);
  analogWrite(PIN_LCD_BL, DEFAULT_BRIGHTNESS);

  // Initialize LCD
  if (!gfx->begin()) {
    Serial.println("[LCD] Error initializing GFX display!");
  } else {
    Serial.println("[LCD] GFX display initialized successfully.");
    // Send ST7789/JD9853 MADCTL (0x36) command: 0x48 sets MX=1 (horizontal flip), BGR=1 (fixes R/B color swap)
    bus->beginWrite();
    bus->writeCommand(0x36); // MADCTL
    bus->write(0x48);        // MX=1 (horizontal flip), MY=0, MV=0, ML=0, BGR=1
    bus->endWrite();
  }

  dashboard.init();
  dashboard.renderHeader("Connecting...", false, false);

  // Touch driver setup
  touchDriver.begin();

  // Establish Wi-Fi
  setupWiFi();

  // Initial fetch & UI render
  refreshPlexMetrics();
}

// ----------------------------------------------------------------------------
// Arduino Loop (Touch & Auto-Poll)
// ----------------------------------------------------------------------------
unsigned long lastTouchTime = 0;

void loop() {
  TouchPoint tp = touchDriver.readTouch();
  
  if (tp.touched) {
    // Register tap debounce (500ms)
    if (millis() - lastTouchTime > 500) {
      lastTouchTime = millis();
      Serial.printf("[TOUCH CONFIRMED] Touch at X=%d, Y=%d -> Manual Refresh\n", tp.x, tp.y);
      
      // Force immediate refresh
      refreshPlexMetrics();
    }
  }

  // Periodic Plex API poll
  if (millis() - lastApiFetchTime > ((unsigned long)API_POLL_INTERVAL_SEC * 1000)) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiStatus = true;
    } else {
      wifiStatus = false;
      WiFi.reconnect();
    }
    refreshPlexMetrics();
  }

  delay(30);
}
