// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>

// Inlcudes for setting up the server
#include <HTTPSServer.hpp>

// Define the certificate data for the server
#include <SSLCert.hpp>

// Includes to define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required do define ResourceNodes
#include <ResourceNode.hpp>

// Own libraries
#include "server.h"
#include "utils.h"

// Access to the classes of the server
using namespace httpsserver;

/* ********************************************************************************************* */

// Logging TAG
const char* TAG = "Server";

// Global instance for server
HTTPSServer *server;

// Define LED pins
const int greenLED = 2;
const int redLED = 4;

int currentLED = -1;
unsigned long ledTimer = 0;
const unsigned long ledOnDuration = 2000;

/* ********************************************************************************************* */

// Handle client requests function
void handleClientRequest(HTTPRequest *req, HTTPResponse *res) {
    // Check if the connection is secure
    if (!req->isSecure()) {
        // Turn on the red LED for a not-secure or suspicious connection
        logMessage(TAG, "Not-secure connection detected, alarm started.");
        digitalWrite(redLED, HIGH);
        return;
    } else {
        // Turn off the red LED if the connection is secure
        digitalWrite(redLED, LOW);
    }

    // Determine the length of the request body
    size_t contentLength = req->getContentLength();

    // Check if there is any request body
    if (contentLength > 0) {
        // Create a character buffer to store the clientKey
        char clientKey[contentLength + 1];
        req->readBytes((byte*)clientKey, contentLength);
        clientKey[contentLength] = '\0';

        // Convert the character buffer to a String
        String clientKeyStr = String(clientKey);

        // Load the stored key from SPIFFS
        String storedKey = readFileFromSPIFFS(TAG, "/secret.txt");

        if (storedKey.isEmpty()) {
            logMessage(TAG, "Failed to open secret.txt");
            res->setStatusCode(500);
            res->println("Error reading server key");
            digitalWrite(redLED, HIGH);
            return;
        }

        // Compare the client key with the stored key
        if (clientKeyStr == storedKey) {
            logMessage(TAG, "Client key matches.");
            res->setStatusCode(200);
            res->println("Success.");
            digitalWrite(greenLED, HIGH);
        } else {
            logMessage(TAG, "Client key does not match.");
            res->setStatusCode(401);
            res->println("Failure");
            digitalWrite(redLED, HIGH);
        }
    } else {
        res->setStatusCode(400);
        res->println("Bad Request: no content.");
    }
}

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

void startServer(int port, bool securityFlag) {
    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        logMessage(TAG, "Failed to mount SPIFFS.");
        return;
    }

    if (securityFlag) {
        // Load server certificate and key in DER format from SPIFFS
        size_t certSize, keySize;
        unsigned char* serverCert = readBinaryFileFromSPIFFS(TAG, "/server_cert.der", &certSize);
        unsigned char* serverKey = readBinaryFileFromSPIFFS(TAG, "/server_key.der", &keySize);

        if (!serverCert || !serverKey) {
            logMessage(TAG, "Error loading certificates or private key.");
            return;
        }

        // Create SSL certificate object using DER format
        SSLCert *cert = new SSLCert(serverCert, certSize, serverKey, keySize);

        // Create HTTPS server using the SSL certificate
        server = new HTTPSServer(cert, port);
        logMessage(TAG, "Secure server started.");

        // Free the memory after creating the certificate
        delete[] serverCert;
        delete[] serverKey;
    } else {
        // Non-secure server: do not load certificates
        server = new HTTPSServer(nullptr, port);
        logMessage(TAG, "Non-secure server started.");
    }

    // Define a resource for the root path
    ResourceNode *node = new ResourceNode("/", "POST", &handleClientRequest);
    server->registerNode(node);

    // Start the server
    server->start();

    // Main loop to handle client requests
    while (true) {
        // Check if the LED needs to be turned off
        if (currentLED != -1 && millis() - ledTimer >= ledOnDuration) {
            // Turn off the LED that was on
            digitalWrite(currentLED, LOW);
            // Reset the current LED indicator
            currentLED = -1;
        }

        // Set the server in Idle mode
        server->loop();

        // Add a small delay to avoid excessive CPU usage
        delay(10);
    }
}

/* ********************************************************************************************* */
