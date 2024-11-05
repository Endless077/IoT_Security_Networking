// Espressif - ESP32 Client (main.cpp)
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <WiFi.h>

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
    static char payload[256];
    strncpy(payload, clientKey.c_str(), sizeof(payload) - 1);
    payload[sizeof(payload) - 1] = '\0';
    
    // HTTP Request
    request = {
        serverPort,             // port
        securityFlag,           // useHTTPS
        "/",                    // uri
        serverAddress,          // host
        "POST",                 // method
        payload,                // payload
        "keep-alive",           // connection
        "text/plain",           // contentType
        "Espressif ESP32/1.0"   // userAgent
    };

}

void loop() {
    // Loop the connection, try a connection every 10 seconds
    logMessage("BOOT", securityFlag ? "Attempting to secure connection..." : "Attempting to not secure connection...");
    sendRequest(request);

    delay(10000);
}

/* ********************************************************************************************* */
