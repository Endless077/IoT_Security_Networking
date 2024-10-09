// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include "client.h"

// WiFiClientSecure for secure connection
WiFiClient client;
WiFiClientSecure secureClient;

/* ********************************************************************************************* */

void setupWiFi(const char* ssid, const char* password) {
    // Begin the WiFi connection process
    WiFi.begin(ssid, password);
    
    // Wait until the ESP32 connects to the WiFi network
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);  // Wait for 1 second
        Serial.println("Connecting to WiFi...");
    }
    
    // Connection successful
    Serial.println("Connected to WiFi");
}

void secureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    // Initialize SPIFFS (file system)
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Load the CA certificate from SPIFFS
    File caFile = SPIFFS.open("/ca.crt", "r");
    if (!caFile) {
        Serial.println("Failed to open CA certificate file");
        return;
    }
    size_t caSize = caFile.size();
    char* caCert = new char[caSize + 1];
    caFile.readBytes(caCert, caSize);
    caCert[caSize] = '\0';
    caFile.close();
    secureClient.setCACert(caCert);
    delete[] caCert;
    Serial.println("CA certificate set");

    // Load the client certificate from SPIFFS
    File clientCertFile = SPIFFS.open("/client.crt", "r");
    if (!clientCertFile) {
        Serial.println("Failed to open client certificate file");
        return;
    }

    size_t certSize = clientCertFile.size();
    char* clientCert = new char[certSize + 1];
    clientCertFile.readBytes(clientCert, certSize);
    clientCert[certSize] = '\0';
    clientCertFile.close();
    secureClient.setCertificate(clientCert);
    delete[] clientCert;
    Serial.println("Client certificate set");

    // Load the client private key from SPIFFS
    File clientKeyFile = SPIFFS.open("/client.key", "r");
    if (!clientKeyFile) {
        Serial.println("Failed to open client key file");
        return;
    }

    size_t keySize = clientKeyFile.size();
    char* clientKey = new char[keySize + 1];
    clientKeyFile.readBytes(clientKey, keySize);
    clientKey[keySize] = '\0';
    clientKeyFile.close();
    secureClient.setPrivateKey(clientKey);
    delete[] clientKey;
    Serial.println("Client private key set");

    // Attempt to establish a secure connection to the server
    if (secureClient.connect(serverAddress, serverPort)) {
        Serial.println("Secure connection established");

        // Send the client's secret key to the server
        secureClient.println(clientSecretKey);

        // Wait for the server's response
        while (secureClient.connected()) {
            if (secureClient.available()) {
                // Read and print the server's response
                String response = secureClient.readStringUntil('\n');
                Serial.println("Secure response from server: " + response);
            }
        }

        // Disconnect from the server once done
        secureClient.stop();
        Serial.println("Secure connection closed");
    } else {
        // If the connection fails, print an error message
        Serial.println("Secure connection to server failed");
    }
}

void notSecureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    // Attempt to connect to the server using an unsecured connection
    if (client.connect(serverAddress, serverPort)) {
        Serial.println("Connected to server");
        
        // Send the client's secret key to the server
        client.println(clientSecretKey);
        
        // Wait for a response from the server
        while (client.connected()) {
            if (client.available()) {
                // Read and print the response from the server
                String response = client.readStringUntil('\n');
                Serial.println("Response from server: " + response);
            }
        }
        
        // Close the connection to the server
        client.stop();
        Serial.println("Disconnected from server");
    } else {
        // Print an error message if the connection fails
        Serial.println("Connection to server failed");
    }
}

/* ********************************************************************************************* */
