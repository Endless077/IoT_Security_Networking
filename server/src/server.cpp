// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>

// Own libraries
#include "server.h"

// Define LED pins
const int greenLED = 2;  // Example pin for green LED
const int redLED = 4;    // Example pin for red LED

// WiFiClientSecure for secure connections
WiFiServer server(80);          // Non-secure server
WiFiServer secureServer(443);   // Secure server

WiFiClient client;
WiFiClientSecure secureClient;

/* ********************************************************************************************* */

String readFileFromSPIFFS(const char* path) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.printf("Failed to open file: %s\n", path);
        return String();
    }
    String content = file.readString();
    file.close();
    return content;
}

/* ********************************************************************************************* */

void setupWiFi(const char* ssid, const char* password) {
    // Begin WiFi connection
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void startSecureServer(int port) {
    // Configure the secure server
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Load the certificates once
    String caCert = readFileFromSPIFFS("/ca.crt");
    String serverCert = readFileFromSPIFFS("/server.crt");
    String serverKey = readFileFromSPIFFS("/server.key");

    if (caCert.isEmpty() || serverCert.isEmpty() || serverKey.isEmpty()) {
        Serial.println("Error loading certificates or private key.");
        return;
    }

    // Use a WiFiClientSecure object directly
    WiFiClientSecure secureClient;
    secureClient.setCACert(caCert.c_str());
    secureClient.setCertificate(serverCert.c_str());
    secureClient.setPrivateKey(serverKey.c_str());

    // Start the server on a secure port
    // Secure server listening on the TLS port
    WiFiServer secureServer(port);
    secureServer.begin();
    Serial.println("Secure server started");

    // Loop to handle secure client connections
    while (true) {
        WiFiClient client = secureServer.available();

        if (client) {
            Serial.println("Client connected on secure server");

            // Ensure the client initiates the TLS handshake
            if (!secureClient.connected()) {
                if (!secureClient.connect(client.remoteIP().toString().c_str(), port)) {
                    Serial.println("Failed to establish secure connection.");
                    client.stop();
                    digitalWrite(redLED, HIGH);
                    continue;
                }
            }

            // Handle the secure connection
            String clientKey = secureClient.readStringUntil('\n');
            clientKey.trim();

            // Load the secret key stored on the server
            String storedKey = readFileFromSPIFFS("/secret.txt");
            if (storedKey.isEmpty()) {
                Serial.println("Failed to open secret.txt");
                secureClient.println("Error reading server key");
                digitalWrite(redLED, HIGH);
            } else {
                // Verify the clientKey
                if (clientKey == storedKey) {
                    Serial.println("Client key matches");
                    secureClient.println("Success");
                    digitalWrite(greenLED, HIGH);
                } else {
                    Serial.println("Client key does not match");
                    secureClient.println("Failure");
                    digitalWrite(redLED, HIGH);
                }
            }

            // Close the secure connection
            secureClient.stop();
            Serial.println("Secure client disconnected");
        }
    }
}

void startNonSecureServer(int port) {
    // Initialize the non-secure server
    server.begin();
    Serial.println("Non-secure server started");

    // Main loop to handle client connections
    while (true) {
        client = server.available();
        if (client) {
            Serial.println("Client connected on non-secure server");

            // Check for client data
            while (client.connected()) {
                if (client.available()) {
                    String clientKey = client.readStringUntil('\n');
                    clientKey.trim();  // Clean the key input

                    // Load the stored client key from SPIFFS
                    File file = SPIFFS.open("/secret.txt", "r");
                    if (!file) {
                        Serial.println("Failed to open secret.txt");
                        client.println("Error reading server key");
                        digitalWrite(redLED, HIGH);  // Turn on red LED
                    } else {
                        String storedKey = file.readStringUntil('\n');
                        file.close();

                        // Validate the client key
                        if (clientKey == storedKey) {
                            Serial.println("Client key matches");
                            client.println("Success");
                            digitalWrite(greenLED, HIGH);  // Turn on green LED
                        } else {
                            Serial.println("Client key does not match");
                            client.println("Failure");
                            digitalWrite(redLED, HIGH);  // Turn on red LED
                        }
                    }
                }
            }
            client.stop();
            Serial.println("Non-secure client disconnected");
        }
    }
}
