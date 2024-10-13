// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>

// Own libraries
#include "server.h"

// Inlcudes for setting up the server
#include <HTTPSServer.hpp>

// Define the certificate data for the server (Certificate and private key)
#include <SSLCert.hpp>

// Includes to define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required do define ResourceNodes
#include <ResourceNode.hpp>

// Easier access to the classes of the server
using namespace httpsserver;

// Define LED pins
const int greenLED = 2;  // Example pin for green LED
const int redLED = 4;    // Example pin for red LED

int currentLED = -1;
unsigned long ledTimer = 0;
const unsigned long ledOnDuration = 2000;


// Global instance for server
HTTPSServer *server;

/* ********************************************************************************************* */

// Function to handle client requests
void handleClientRequest(HTTPRequest *req, HTTPResponse *res) {
    // Check if the connection is secure
    if (!req->isSecure()) {
        // Turn on the red LED for a not-secure or suspicious connection
        Serial.println("Not-secure connection detected, alarm started.");
        digitalWrite(redLED, HIGH);
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
        String storedKey = readFileFromSPIFFS("/secret.txt");

        if (storedKey.isEmpty()) {
            Serial.println("Failed to open secret.txt");
            res->setStatusCode(500);
            res->println("Error reading server key");
            digitalWrite(redLED, HIGH);
            return;
        }

        // Compare the client key with the stored key
        if (clientKeyStr == storedKey) {
            Serial.println("Client key matches");
            res->setStatusCode(200);
            res->println("Success");
            digitalWrite(greenLED, HIGH);
        } else {
            Serial.println("Client key does not match");
            res->setStatusCode(401);
            res->println("Failure");
            digitalWrite(redLED, HIGH);
        }
    } else {
        res->setStatusCode(400);  // Return error if no request body is present
        res->println("Bad Request: no content");
    }
}

// SPIFFS Support Functions
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

unsigned char* readBinaryFileFromSPIFFS(const char* path, size_t* fileSize) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.printf("Failed to open file: %s\n", path);
        return nullptr;
    }
    *fileSize = file.size();
    unsigned char* buffer = new unsigned char[*fileSize];
    file.read(buffer, *fileSize);
    file.close();
    return buffer;
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

void startServer(int port, bool useSecureConnection) {
    // Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    if (useSecureConnection) {
        // Load server certificate and key in DER format from SPIFFS
        size_t certSize, keySize;
        unsigned char* serverCert = readBinaryFileFromSPIFFS("/server_cert.der", &certSize);
        unsigned char* serverKey = readBinaryFileFromSPIFFS("/server_key.der", &keySize);

        if (!serverCert || !serverKey) {
            Serial.println("Error loading certificates or private key.");
            return;
        }

        // Create SSL certificate object using DER format
        SSLCert *cert = new SSLCert(serverCert, certSize, serverKey, keySize);

        // Create HTTPS server using the SSL certificate
        server = new HTTPSServer(cert, port);
        Serial.println("Secure server started");

        // Free the memory after creating the certificate
        delete[] serverCert;
        delete[] serverKey;
    } else {
        // Non-secure server: do not load certificates
        server = new HTTPSServer(nullptr, port);
        Serial.println("Non-secure server started");
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
