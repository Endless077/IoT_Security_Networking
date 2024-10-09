// Espressif - ESP32 Client (main.cpp)
#include <WiFi.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"

// Flag to determine if the connection should be secure or not
bool useSecureConnection = true;

// Variable to hold the client secret key read from the file
String clientKey = "";

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server IP address (ESP32)
const char* serverAddress = "IP_ADDRESS";

// Server port (will be determined in the setup function)
int serverPort;

void setup() {
    Serial.begin(115200);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    // Read the secret key from the secret.txt file
    File file = SPIFFS.open("/secret.txt", "r");
    if (!file) {
        Serial.println("Failed to open secret.txt file");
    } else {
        size_t keySize = file.size();
        char* clientKey = new char[keySize + 1];
        file.readBytes(clientKey, keySize);
        clientKey[keySize] = '\0';
        file.close();
        Serial.println("Secret key loaded from file: " + String(clientKey));

        // Connect to the WiFi network
        setupWiFi(ssid, password);

        // Determine the server port based on the connection type
        if (!useSecureConnection) {
            serverPort = 80;
            Serial.println("Attempting non-secure connection...");
            notSecureConnection(serverAddress, serverPort, clientKey);  // Non-secure connection
        } else {
          serverPort = 443;
          Serial.println("Attempting secure connection...");
          secureConnection(serverAddress, serverPort, clientKey);       // Secure connection
        }

        // Free the memory allocated for the secret key
        delete[] clientKey;
    }
}

void loop() {
  // The client may reconnect after a delay
  // Wait 10 seconds before the next attempt
  delay(10000);
}
