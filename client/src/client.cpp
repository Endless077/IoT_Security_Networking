// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>

// Own libraries
#include "client.h"
#include "utils.h"

/* ********************************************************************************************* */

const char* LOG = "Client";

WiFiClient client;
WiFiClientSecure secureClient;

/* ********************************************************************************************* */

void setupWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    logMessage(LOG, "Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logMessage(LOG, ".");
    }

    logMessage(LOG, (String("IP Address: ") + WiFi.localIP().toString()).c_str());
    logMessage(LOG, "Connected to WiFi.");
}

void secureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    // Initialize SPIFFS (file system)
    if (!SPIFFS.begin(true)) {
        logMessage(LOG, "Failed to mount SPIFFS.");
        return;
    }

    // Load the CA certificate manually from SPIFFS
    File caFile = SPIFFS.open("/ca.crt", "r");
    if (!caFile) {
        logMessage(LOG, "Failed to open CA certificate file.");
        return;
    }
    String caCert = caFile.readString();
    caFile.close();

    // Set the CA certificate
    secureClient.setCACert(caCert.c_str());
    logMessage(LOG, "CA certificate loaded");

    // Load the client certificate manually from SPIFFS
    File clientCertFile = SPIFFS.open("/client.crt", "r");
    if (!clientCertFile) {
        logMessage(LOG, "Failed to open client certificate file.");
        return;
    }
    String clientCert = clientCertFile.readString();
    clientCertFile.close();

    // Set the client certificate
    secureClient.setCertificate(clientCert.c_str());
    logMessage(LOG, "Client certificate loaded.");

    // Load the client key manually from SPIFFS
    File clientKeyFile = SPIFFS.open("/client.key", "r");
    if (!clientKeyFile) {
        logMessage(LOG, "Failed to open client key file.");
        return;
    }
    String clientKey = clientKeyFile.readString();
    clientKeyFile.close();

    // Set the client private key
    secureClient.setPrivateKey(clientKey.c_str());
    logMessage(LOG, "Client private key loaded.");

    // Attempt to establish a secure connection
    if (secureClient.connect(serverAddress, serverPort)) {
        logMessage(LOG, "Secure connection established.");
        secureClient.println(clientSecretKey);
        while (secureClient.connected()) {
            if (secureClient.available()) {
                String response = secureClient.readStringUntil('\n');
                logMessage(LOG, (String("Secure response from server: ") + response).c_str());
            }
        }
        secureClient.stop();
        logMessage(LOG, "Secure connection closed.");
    } else {
        logMessage(LOG, "Secure connection failed.");
    }
}

void notSecureConnection(const char* serverAddress, int serverPort, const char* clientSecretKey) {
    if (client.connect(serverAddress, serverPort)) {
        logMessage(LOG, "Connected to server.");
        client.println(clientSecretKey);
        while (client.connected()) {
            if (client.available()) {
                String response = client.readStringUntil('\n');
                logMessage(LOG, (String("Response from server: ") + response).c_str());
            }
        }
        client.stop();
        logMessage(LOG, "Disconnected from server.");
    } else {
        logMessage(LOG, "Connection to server failed.");
    }
}

/* ********************************************************************************************* */
