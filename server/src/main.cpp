// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "server.h"

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Flag to determine if the connection should be secure or not
bool useSecureConnection = true;
#include <WiFi.h>
#include <SPIFFS.h>
#include "server.h"

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Flag to determine if the connection should be secure or not
bool useSecureConnection = true;

void setup() {
    Serial.begin(115200);

    // Setup LED pins
    pinMode(2, OUTPUT);
    pinMode(4, OUTPUT);
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Connect to WiFi
    setupWiFi(ssid, password);

    // Start the server (secure or non-secure based on the flag)
    startServer(useSecureConnection ? 443 : 80, useSecureConnection);
}

void loop() {}
