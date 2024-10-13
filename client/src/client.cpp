// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"
#include "utils.h"

/* ********************************************************************************************* */

// Logging TAG
const char* TAG = "Client";

// WiFiClient for secure and not secure connection
WiFiClient client;
WiFiClientSecure secureClient;

/* ********************************************************************************************* */

void setupWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    logMessage(TAG, "Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logMessage(TAG, ".");
    }
    logMessage(TAG, "\nConnected to WiFi.");
}

void secureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    // Initialize SPIFFS (file system)
    if (!SPIFFS.begin(true)) {
        logMessage(TAG, "Failed to mount SPIFFS.");
        return;
    }

    // Load the CA certificate manually from SPIFFS
    File caFile = SPIFFS.open("/ca.crt", "r");
    if (!caFile) {
        logMessage(TAG, "Failed to open CA certificate file.");
        return;
    }
    String caCert = caFile.readString();
    caFile.close();

    // Set the CA certificate
    secureClient.setCACert(caCert.c_str());
    logMessage(TAG, "CA certificate loaded");

    // Load the client certificate manually from SPIFFS
    File clientCertFile = SPIFFS.open("/client.crt", "r");
    if (!clientCertFile) {
        logMessage(TAG, "Failed to open client certificate file.");
        return;
    }
    String clientCert = clientCertFile.readString();
    clientCertFile.close();

    // Set the client certificate
    secureClient.setCertificate(clientCert.c_str());
    logMessage(TAG, "Client certificate loaded.");

    // Load the client key manually from SPIFFS
    File clientKeyFile = SPIFFS.open("/client.key", "r");
    if (!clientKeyFile) {
        logMessage(TAG, "Failed to open client key file.");
        return;
    }
    String clientKey = clientKeyFile.readString();
    clientKeyFile.close();

    // Set the client private key
    secureClient.setPrivateKey(clientKey.c_str());
    logMessage(TAG, "Client private key loaded.");

    // Attempt to establish a secure connection
    if (secureClient.connect(serverAddress, serverPort)) {
        logMessage(TAG, "Secure connection established.");
        secureClient.println(clientSecretKey);
        while (secureClient.connected()) {
            if (secureClient.available()) {
                String response = secureClient.readStringUntil('\n');
                logMessage(TAG, (String("Secure response from server: ") + response).c_str());
            }
        }
        secureClient.stop();
        logMessage(TAG, "Secure connection closed.");
    } else {
        logMessage(TAG, "Secure connection failed.");
    }
}

void notSecureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    if (client.connect(serverAddress, serverPort)) {
        logMessage(TAG, "Connected to server.");
        client.println(clientSecretKey);
        while (client.connected()) {
            if (client.available()) {
                String response = client.readStringUntil('\n');
                logMessage(TAG, (String("Response from server: ") + response).c_str());
            }
        }
        client.stop();
        logMessage(TAG, "Disconnected from server.");
    } else {
        logMessage(TAG, "Connection to server failed.");
    }
}

/* ********************************************************************************************* */
