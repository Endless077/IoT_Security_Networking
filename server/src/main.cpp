// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "server.h"
#include "utils.h"

// Determine if the connection should be secure or not
bool securityFlag = false;

/* ********************************************************************************************* */

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server port
int serverPort = securityFlag ? 443 : 80;

/* ********************************************************************************************* */

void setup() {
    //Init setup()
    logMessage("BOOT", "Init setup()");
    Serial.begin(115200);

    // Setup LED pins
    logMessage("BOOT", "Setup the digitla Pins");
    pinMode(2, OUTPUT);
    pinMode(13, OUTPUT);
    digitalWrite(2, LOW);
    digitalWrite(13, LOW);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
      logMessage("BOOT", "Failed to mount SPIFFS.");
      return;
    }

    // Connect to WiFi
    logMessage("BOOT", "Setup WiFi Connection.");
    setupWiFi(ssid, password);

    // Start the server (secure or non-secure based on the flag)
    logMessage("BOOT", "Setup Server Boot.");
    startServer(serverPort, securityFlag);
}

void loop() {
  // Check and reset the service
  resetService();

  // Set the server in Idle mode
  if (securityFlag) {
    // Server HTTPS
    if (serverHTTPS != nullptr && serverHTTPS->isRunning()) {
      serverHTTPS->loop();
    } else {
      logMessage("BOOT", "Server HTTPS non disponibile.");
    }
  } else {
    // Server HTTP
    if (serverHTTP != nullptr && serverHTTP->isRunning()) {
      serverHTTP->loop();
    } else {
      logMessage("BOOT", "Server HTTP non disponibile.");
      }
    }

  // Add a small delay to avoid excessive CPU usage
  delay(10);
}

/* ********************************************************************************************* */