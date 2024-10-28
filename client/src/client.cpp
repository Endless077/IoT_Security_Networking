// Espressif - ESP32 Client (client.cpp)
#include <ESPmDNS.h>
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

void secureConnection(const HttpRequest& request) {
    // Initialize SPIFFS (file system)
    if (!SPIFFS.begin(true)) {
        logMessage(LOG, "Failed to mount SPIFFS.");
        return;
    }

    // Load the CA certificate manually from SPIFFS
    String caCert = readFileFromSPIFFS("/ca_cert.pem");

    // Set the CA certificate
    secureClient.setCACert(caCert.c_str());
    logMessage(LOG, "CA certificate loaded");

    // Load the client certificate manually from SPIFFS
    String clientCert = readFileFromSPIFFS("/client_cert.pem");

    // Set the client certificate
    //secureClient.setCertificate(clientCert.c_str());
    logMessage(LOG, "Client certificate loaded.");

    // Load the client key manually from SPIFFS
    String clientKey = readFileFromSPIFFS("/client_key.pem");

    // Set the client private key
    //secureClient.setPrivateKey(clientKey.c_str());
    logMessage(LOG, "Client private key loaded.");

    // Attempt to establish a secure connection
    if (secureClient.connect(request.host, request.port)) {
        logMessage(LOG, (String("Secure connection established to: ") + request.host).c_str());
        
        // Header Setup
        char headerBuffer[256];
        int bodyLength = String(request.body).length();

        snprintf(headerBuffer, sizeof(headerBuffer),
                 "%s %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n",
                 request.method,
                 request.path,
                 request.host,
                 request.contentType,
                 bodyLength);

        // Send the Header
        secureClient.print(headerBuffer);

        // Send the Body
        if (bodyLength > 0) {
            secureClient.print(request.body);
        }

        // Wait a server response
        while (secureClient.connected()) {
            if (secureClient.available()) {
                String response = secureClient.readStringUntil('\n');
                response.trim();
                logMessage(LOG, (String("Secure response from server: ") + response).c_str());
            }
        }
        secureClient.stop();
        logMessage(LOG, "Connection to Secure server closed.");
    } else {
        logMessage(LOG, "Connection to Secure server failed.");
    }
}

void notSecureConnection(const HttpRequest& request) {
    if (client.connect(request.host, request.port)) {
        logMessage(LOG, (String("Unsecure connecton established to: ") + request.host).c_str());

        // Header Setup
        char headerBuffer[256];
        int bodyLength = String(request.body).length();

        snprintf(headerBuffer, sizeof(headerBuffer),
                 "%s %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n\r\n",
                 request.method,
                 request.path,
                 request.host,
                 request.contentType,
                 bodyLength);

        // Send the Header
        client.print(headerBuffer);

        // Send the Body
        if (bodyLength > 0) {
            client.print(request.body);
        }

        // Wait a server response
        while (client.connected()) {
            if (client.available()) {
                String response = client.readStringUntil('\n');
                response.trim();
                if (response.length() > 0) {
                    logMessage(LOG, (String("Response from server: ") + response).c_str());
                }
            }
        }
        client.stop();
        logMessage(LOG, "Connection to unsecure server closed.");
    } else {
        logMessage(LOG, "Connection to unsecure server failed.");
    }
}

void setupWiFi(const char* ssid, const char* password) {
    // Check a WiFi connection with given SSID and Password
    WiFi.begin(ssid, password);
    logMessage(LOG, "Connecting to WiFi");
    
    // Start the WiFi connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logMessage(LOG, ".");
    }

    // Get the IP Address of the server
    logMessage(LOG, (String("IP Address: ") + WiFi.localIP().toString()).c_str());
    logMessage(LOG, "Connected to WiFi.");

    // Start mDNS
    if (!MDNS.begin("esp32client")) {
        logMessage(LOG, "Error setting up MDNS responder.");
        while (1) {
            delay(1000);
        }
    }
    logMessage(LOG, "mDNS responder started.");

    // Configure time with NTP servers
    configTime(0, 3600, "pool.ntp.org", "time.nist.gov");

    // Wait until time is synchronized
    logMessage(LOG, "Waiting for time synchronization...");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.print(".");
        delay(1000);
    }

    logMessage(LOG, "Time synchronized.");
    
    // Stamp the current time
    char timeString[64];
    strftime(timeString, sizeof(timeString), "Current time: %A, %B %d %Y %H:%M:%S", &timeinfo);
    logMessage(LOG, timeString);
}

/* ********************************************************************************************* */
