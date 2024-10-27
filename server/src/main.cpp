// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// HTTPServer Class
#include <HTTPSServer.hpp>

// Own libraries
#include "server.h"
#include "utils.h"

// Determine if the connection should be secure or not
bool securityFlag = true;

/* ********************************************************************************************* */

// WiFi credentials
const char* ssid = "";
const char* password = "";

// Server port
int serverPort = securityFlag ? 443 : 80;

/* ********************************************************************************************* */

void setup() {
    //Init setup()
    logMessage("BOOT", "Init setup()");
    Serial.begin(115200);

    // Setup LED pins
    logMessage("BOOT", "Setup the digital Pins");
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
  // Check and Reset service
  resetService();

  // Set the server in Idle mode
  if (!securityFlag) {
    // Non-secure connection
    logMessage("BOOT", "Attempting non-secure connection...");
    if (serverHTTP != nullptr && serverHTTP->isRunning()) {
        serverHTTP->loop();
    } else {
        logMessage("BOOT", "Server HTTP not available.");
    }
  } else {
    // Secure connection
    logMessage("BOOT", "Attempting secure connection...");
    if (serverHTTPS != nullptr && serverHTTPS->isRunning()) {
        serverHTTPS->loop();
    } else {
        logMessage("BOOT", "Server HTTPS not available.");
    }
  }

  // Add a small delay to avoid excessive CPU usage
  delay(1000);
}

/* ********************************************************************************************* */