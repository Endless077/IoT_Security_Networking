// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"
#include "utils.h"

// Determine if the connection should be secure or not
bool securityFlag = false;

/* ********************************************************************************************* */

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server IP address
const char* serverAddress = "IP_ADDRESS";

// Server port
int serverPort = securityFlag ? 443 : 80;

// Request
HttpRequest request;

/* ********************************************************************************************* */

void setup() {
    // Init setup()
    Serial.begin(115200);
    logMessage("BOOT", "Init setup()");

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        logMessage("BOOT", "An error occurred while mounting SPIFFS");
        return;
    }

    // Read the secret key from the secret.txt file
    String clientKey = readFileFromSPIFFS("/secret.txt");

    logMessage("BOOT", (String("Client key retrieved: ") + clientKey).c_str());

    // Setup the body
    static char bodyBuffer[256];
    strncpy(bodyBuffer, clientKey.c_str(), sizeof(bodyBuffer) - 1);
    bodyBuffer[sizeof(bodyBuffer) - 1] = '\0';

    // HTTP Request
    request = {
        serverPort,             // port
        securityFlag,           // useHttps
        "POST",                 // method
        "/",                    // path
        serverAddress,          // host
        "text/plain",           // contentType
        bodyBuffer,             // body
    };

    // Connect to the WiFi network
    setupWiFi(ssid, password);
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
