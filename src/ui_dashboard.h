#ifndef UI_DASHBOARD_H
#define UI_DASHBOARD_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "plex_client.h"

// Color definitions (16-bit RGB565) - Pure Obsidian Dark Theme with Plex Amber Accents
#define COLOR_BG          0x0000  // Pure Obsidian Black
#define COLOR_CARD_BG     0x0000  // Pure Black Container Background
#define COLOR_CARD_BORDER 0x2104  // Dark Charcoal Border Accent (RGB: 32, 32, 32)
#define COLOR_TEXT_WHITE  0xFFFF  // Crisp White
#define COLOR_TEXT_MUTED  0xAD75  // Soft Muted Gray
#define COLOR_PLEX_AMBER  0xFFE0  // Bright Plex Amber / Yellow
#define COLOR_GREEN       0x2E64  // Emerald Green (Online)
#define COLOR_RED         0xF980  // Crimson Red (Error)

class UIDashboard {
private:
  Arduino_GFX *gfx;

public:
  UIDashboard(Arduino_GFX *display) : gfx(display) {}

  void init() {
    gfx->fillScreen(COLOR_BG);
  }

  void renderHeader(const String& serverName, bool wifiConnected, bool serverOnline) {
    gfx->fillRect(0, 0, 172, 28, COLOR_BG);
    
    // Status Dot: Green if Wi-Fi + Server Online, Red if offline
    uint16_t statusColor = (wifiConnected && serverOnline) ? COLOR_GREEN : COLOR_RED;
    gfx->fillCircle(12, 14, 4, statusColor);

    // Title Header
    gfx->setTextColor(COLOR_TEXT_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(24, 6);
    gfx->print("PLEX MONITOR");

    // Header divider line
    gfx->drawFastHLine(0, 28, 172, COLOR_CARD_BORDER);
  }

  void renderMainStreamerCard(const PlexMetrics& metrics) {
    int cardY = 34;
    int cardH = 118;
    
    // Container Box - Black Background with Subtle Border
    gfx->fillRoundRect(6, cardY, 160, cardH, 8, COLOR_CARD_BG);
    gfx->drawRoundRect(6, cardY, 160, cardH, 8, COLOR_CARD_BORDER);

    // Label
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT_MUTED);
    gfx->setCursor(14, cardY + 10);
    gfx->print("ACTIVE STREAMERS");

    if (metrics.serverOnline) {
      // Big prominent Streamer Count in Yellow / Amber always
      gfx->setTextSize(6);
      gfx->setTextColor(COLOR_PLEX_AMBER);
      gfx->setCursor(14, cardY + 28);
      gfx->print(metrics.activeStreamers);

      // Status Subtitle
      gfx->setTextSize(1);
      gfx->setCursor(14, cardY + 92);
      if (metrics.activeStreamers == 1) {
        gfx->setTextColor(COLOR_PLEX_AMBER);
        gfx->print("1 Active Session");
      } else if (metrics.activeStreamers > 1) {
        gfx->setTextColor(COLOR_PLEX_AMBER);
        gfx->printf("%d Active Sessions", metrics.activeStreamers);
      } else {
        gfx->setTextColor(COLOR_TEXT_MUTED);
        gfx->print("Server Idle (0 streams)");
      }
    } else {
      // Offline / Error Display
      gfx->setTextSize(3);
      gfx->setTextColor(COLOR_RED);
      gfx->setCursor(14, cardY + 36);
      gfx->print("OFFLINE");

      gfx->setTextSize(1);
      gfx->setTextColor(COLOR_TEXT_MUTED);
      gfx->setCursor(14, cardY + 92);
      if (metrics.errorMessage.length() > 0) {
        // Truncate long error message to fit screen
        String err = metrics.errorMessage;
        if (err.length() > 22) err = err.substring(0, 22);
        gfx->print(err);
      } else {
        gfx->print("Check Connection");
      }
    }
  }

  void renderSessionDetailsCard(const PlexMetrics& metrics) {
    int cardY = 158;
    int cardH = 114;

    gfx->fillRoundRect(6, cardY, 160, cardH, 8, COLOR_CARD_BG);
    gfx->drawRoundRect(6, cardY, 160, cardH, 8, COLOR_CARD_BORDER);

    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_PLEX_AMBER);
    gfx->setCursor(14, cardY + 10);
    gfx->print("STREAM DETAILS");

    if (metrics.serverOnline && metrics.activeStreamers > 0) {
      int yOffset = cardY + 28;
      for (int i = 0; i < metrics.sessionCount && i < 3; i++) {
        // Line 1: User & State
        gfx->setTextColor(COLOR_TEXT_WHITE);
        gfx->setCursor(14, yOffset);
        String userStr = metrics.sessions[i].username;
        if (userStr.length() > 14) userStr = userStr.substring(0, 14);
        gfx->print(userStr);

        // State Pill/Text (e.g. playing/paused)
        gfx->setTextColor(COLOR_TEXT_MUTED);
        gfx->setCursor(110, yOffset);
        String state = metrics.sessions[i].playerState;
        if (state == "playing") gfx->setTextColor(COLOR_GREEN);
        else if (state == "paused") gfx->setTextColor(COLOR_PLEX_AMBER);
        gfx->print(state);

        // Line 2: Media Title
        yOffset += 12;
        gfx->setTextColor(COLOR_TEXT_MUTED);
        gfx->setCursor(14, yOffset);
        String titleStr = metrics.sessions[i].mediaTitle;
        if (titleStr.length() > 22) titleStr = titleStr.substring(0, 20) + "..";
        gfx->print(titleStr);

        yOffset += 16; // Gap before next item
      }
    } else if (metrics.serverOnline) {
      gfx->setTextColor(COLOR_TEXT_MUTED);
      gfx->setCursor(14, cardY + 40);
      gfx->print("No active streams.");
      gfx->setCursor(14, cardY + 56);
      gfx->print("Plex server is ready.");
    } else {
      gfx->setTextColor(COLOR_TEXT_MUTED);
      gfx->setCursor(14, cardY + 36);
      gfx->print("Config Host & Token:");
      gfx->setTextColor(COLOR_TEXT_WHITE);
      gfx->setCursor(14, cardY + 54);
      gfx->print(PLEX_SERVER_HOST);
      gfx->setCursor(14, cardY + 70);
      gfx->printf("Port: %d", PLEX_SERVER_PORT);
    }
  }

  void renderFooterBar(const String& lastSyncMsg) {
    int footerY = 278;
    gfx->fillRect(0, footerY, 172, 42, COLOR_BG);

    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT_MUTED);
    gfx->setCursor(12, footerY + 6);
    gfx->print("Tap screen to refresh");

    gfx->setTextColor(COLOR_PLEX_AMBER);
    gfx->setCursor(12, footerY + 22);
    gfx->print(lastSyncMsg);
  }
};

#endif // UI_DASHBOARD_H
