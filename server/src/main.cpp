// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "server.h"
#include "utils.h"

// Determine if the connection should be secure or not
bool securityFlag = true;

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
    pinMode(4, OUTPUT);
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);

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
    // The server may reconnect after a delay
    // Wait 10 seconds before the next attempt
    logMessage("BOOT", "loop()");
    delay(10000);
    return;
}

/* ********************************************************************************************* */