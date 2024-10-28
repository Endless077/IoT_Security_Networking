// Espressif - ESP32 Client (main.cpp)
#include <ESPmDNS.h>
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"
#include "utils.h"

// Determine if the connection should be secure or not
bool securityFlag = true;

/* ********************************************************************************************* */

// WiFi Credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server credentials
int serverPort = securityFlag ? 443 : 80;
const char* serverAddress = "esp32server.local";

// Request
HttpRequest request;

/* ********************************************************************************************* */

void setup() {
    // Init setup()
    Serial.begin(115200);
    logMessage("BOOT", "Init setup()");

    // Connect to the WiFi network
    setupWiFi(ssid, password);

    // SPIFFS File System Init
    if (!SPIFFS.begin(true)) {
        logMessage("BOOT", "An error occurred while mounting SPIFFS.");
        ESP.restart();
    }

    // Read the secret key from the secret.txt file
    String clientKey = readFileFromSPIFFS("/secret.txt");

    logMessage("BOOT", (String("Client key retrieved: ") + clientKey).c_str());
    
    // Setup the client key
    static char bodyBuffer[256];
    strncpy(bodyBuffer, clientKey.c_str(), sizeof(bodyBuffer) - 1);
    bodyBuffer[sizeof(bodyBuffer) - 1] = '\0';
    
    // HTTP Request
    request = {
        serverPort,     // port
        serverAddress,  // host
        "POST",         // method
        "/",            // path
        "text/plain",   // contentType
        bodyBuffer,     // body
    };

}

void loop() {
    // Wait 10 seconds before the next attempt
    logMessage("BOOT", "Attempting to connect to the server...");

    // Determine the server port based on the connection type
    if (!securityFlag) {
        // Non-secure connection
        logMessage("BOOT", "Attempting non-secure connection...");
        notSecureConnection(request);
    } else {
        // Secure connection
        logMessage("BOOT", "Attempting secure connection...");
        secureConnection(request);
    }

    delay(10000);
}

/* ********************************************************************************************* */
