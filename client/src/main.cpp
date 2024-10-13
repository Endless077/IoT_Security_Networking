// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"
#include "utils.h"

// Determine if the connection should be secure or not
bool securityFlag = true;

// Variable to hold the client secret key read from the file
String clientKey = "";

/* ********************************************************************************************* */

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server IP address
const char* serverAddress = "IP_ADDRESS";

// Server port
int serverPort = securityFlag ? 443 : 80;

/* ********************************************************************************************* */

void setup() {
    //Init setup()
    logMessage("BOOT", "Init setup()");
    Serial.begin(115200);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        logMessage("BOOT", "An error occurred while mounting SPIFFS");
        return;
    }

    // Read the secret key from the secret.txt file
    File file = SPIFFS.open("/secret.txt", "r");
    if (!file) {
       logMessage("BOOT", "Failed to open secret.txt file.");
    } else {
        size_t keySize = file.size();
        char* clientKey = new char[keySize + 1];
        file.readBytes(clientKey, keySize);
        clientKey[keySize] = '\0';
        file.close();
        logMessage("BOOT", (String("Secure response from server: ") + String(clientKey)).c_str());

        // Connect to the WiFi network
        setupWiFi(ssid, password);

        // Determine the server port based on the connection type
        if (!securityFlag) {
            // Non-secure connection
            logMessage("BOOT", "Attempting non-secure connection...");
            notSecureConnection(serverAddress, serverPort, clientKey);
        } else {
            // Secure connection
            logMessage("BOOT", "Attempting secure connection...");
            secureConnection(serverAddress, serverPort, clientKey);
        }

        // Free the memory allocated for the secret key
        delete[] clientKey;
    }
}

void loop() {
    // The client may reconnect after a delay
    // Wait 10 seconds before the next attempt
    logMessage("BOOT", "loop()");
    delay(10000);
    return;
}

/* ********************************************************************************************* */