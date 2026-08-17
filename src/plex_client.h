#ifndef PLEX_CLIENT_H
#define PLEX_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

struct PlexSession {
  String username;
  String mediaTitle;
  String playerState; // "playing", "paused", "buffering"
};

struct PlexMetrics {
  int activeStreamers;
  bool serverOnline;
  String serverName;
  String errorMessage;
  PlexSession sessions[3]; // Up to top 3 session details for UI display
  int sessionCount;
};

class PlexClient {
public:
  static PlexMetrics fetchMetrics(const String& host, int port, const String& token, bool useHttps) {
    PlexMetrics metrics;
    metrics.activeStreamers = 0;
    metrics.serverOnline = false;
    metrics.serverName = "Plex Media Server";
    metrics.sessionCount = 0;
    metrics.errorMessage = "";

    if (WiFi.status() != WL_CONNECTED) {
      metrics.errorMessage = "WiFi Offline";
      return metrics;
    }

    HTTPClient http;
    String protocol = useHttps ? "https://" : "http://";
    String url = protocol + host + ":" + String(port) + "/status/sessions";
    if (token.length() > 0 && token != "YOUR_PLEX_TOKEN_HERE") {
      url += "?X-Plex-Token=" + token;
    }

    if (useHttps) {
      WiFiClientSecure client;
      client.setInsecure();
      http.begin(client, url);
      http.addHeader("Accept", "application/json");
      http.setTimeout(5000);
      executeFetch(http, metrics);
      http.end();
    } else {
      WiFiClient client;
      http.begin(client, url);
      http.addHeader("Accept", "application/json");
      http.setTimeout(5000);
      executeFetch(http, metrics);
      http.end();
    }

    return metrics;
  }

private:
  static void executeFetch(HTTPClient& http, PlexMetrics& metrics) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, payload);

      if (!err) {
        metrics.serverOnline = true;
        JsonObject container = doc["MediaContainer"];
        
        if (container.containsKey("name") && !container["name"].isNull()) {
          metrics.serverName = container["name"].as<String>();
        }

        int size = container["size"] | 0;
        metrics.activeStreamers = size;

        JsonArray metadata = container["Metadata"].as<JsonArray>();
        int count = 0;
        for (JsonObject s : metadata) {
          if (count >= 3) break; // Limit to 3 items for display

          PlexSession session;
          
          // Extract User Name
          if (!s["User"]["title"].isNull()) {
            session.username = s["User"]["title"].as<String>();
          } else {
            session.username = "Guest";
          }

          // Extract Media Title (TV Show or Movie)
          if (!s["grandparentTitle"].isNull()) {
            session.mediaTitle = s["grandparentTitle"].as<String>();
          } else if (!s["title"].isNull()) {
            session.mediaTitle = s["title"].as<String>();
          } else {
            session.mediaTitle = "Media Stream";
          }

          // Extract Player State
          if (!s["Player"]["state"].isNull()) {
            session.playerState = s["Player"]["state"].as<String>();
          } else {
            session.playerState = "playing";
          }

          metrics.sessions[count++] = session;
        }
        metrics.sessionCount = count;

      } else {
        metrics.errorMessage = "JSON Parse Err";
      }
    } else if (httpCode == 401 || httpCode == 403) {
      metrics.errorMessage = "Invalid Token";
    } else if (httpCode > 0) {
      metrics.errorMessage = "HTTP Error " + String(httpCode);
    } else {
      metrics.errorMessage = "Server Unreachable";
    }
  }
};

#endif // PLEX_CLIENT_H
