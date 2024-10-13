// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"

// WiFiClient for secure and not secure connection
WiFiClient client;
WiFiClientSecure secureClient;

void setupWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

void secureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    // Initialize SPIFFS (file system)
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Load the CA certificate manually from SPIFFS
    File caFile = SPIFFS.open("/ca.crt", "r");
    if (!caFile) {
        Serial.println("Failed to open CA certificate file");
        return;
    }
    String caCert = caFile.readString();
    caFile.close();

    // Set the CA certificate
    secureClient.setCACert(caCert.c_str());
    Serial.println("CA certificate loaded");

    // Load the client certificate manually from SPIFFS
    File clientCertFile = SPIFFS.open("/client.crt", "r");
    if (!clientCertFile) {
        Serial.println("Failed to open client certificate file");
        return;
    }
    String clientCert = clientCertFile.readString();
    clientCertFile.close();

    // Set the client certificate
    secureClient.setCertificate(clientCert.c_str());
    Serial.println("Client certificate loaded");

    // Load the client key manually from SPIFFS
    File clientKeyFile = SPIFFS.open("/client.key", "r");
    if (!clientKeyFile) {
        Serial.println("Failed to open client key file");
        return;
    }
    String clientKey = clientKeyFile.readString();
    clientKeyFile.close();

    // Set the client private key
    secureClient.setPrivateKey(clientKey.c_str());
    Serial.println("Client private key loaded");

    // Attempt to establish a secure connection
    if (secureClient.connect(serverAddress, serverPort)) {
        Serial.println("Secure connection established");
        secureClient.println(clientSecretKey);
        while (secureClient.connected()) {
            if (secureClient.available()) {
                String response = secureClient.readStringUntil('\n');
                Serial.println("Secure response from server: " + response);
            }
        }
        secureClient.stop();
        Serial.println("Secure connection closed");
    } else {
        Serial.println("Secure connection failed");
    }
}

void notSecureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    if (client.connect(serverAddress, serverPort)) {
        Serial.println("Connected to server");
        client.println(clientSecretKey);
        while (client.connected()) {
            if (client.available()) {
                String response = client.readStringUntil('\n');
                Serial.println("Response from server: " + response);
            }
        }
        client.stop();
        Serial.println("Disconnected from server");
    } else {
        Serial.println("Connection to server failed");
    }
}
