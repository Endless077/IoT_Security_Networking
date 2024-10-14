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

// Define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required for ResourceNodes definition
#include <ResourceNode.hpp>

// Own libraries
#include "server.h"
#include "utils.h"

// Using namespace
using namespace httpsserver;

/* ********************************************************************************************* */

const char* LOG = "Server";
HTTPServer *serverHTTP = nullptr;
HTTPSServer *serverHTTPS = nullptr;

/* ********************************************************************************************* */

// Handle client requests function
void handleClientRequest(HTTPRequest *req, HTTPResponse *res) {
    // Check if the connection is secure
    if (!req->isSecure()) {
        // Turn on the red LED for a not-secure or suspicious connection
        logMessage(LOG, "Not-secure connection detected, alarm started.");
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
        String storedKey = readFileFromSPIFFS(LOG, "/secret.txt");

        if (storedKey.isEmpty()) {
            logMessage(LOG, "Failed to open secret.txt");
            res->setStatusCode(500);
            res->println("Error reading server key");
            digitalWrite(redLED, HIGH);
            return;
        }

        // Compare the client key with the stored key
        if (clientKeyStr == storedKey) {
            logMessage(LOG, "Client key matches.");
            res->setStatusCode(200);
            res->println("Success.");
            digitalWrite(greenLED, HIGH);
        } else {
            logMessage(LOG, "Client key does not match.");
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
    logMessage(LOG, "Connecting to WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logMessage(LOG, ".");
    }

    logMessage(LOG, (String("IP Address: ") + WiFi.localIP().toString()).c_str());
    logMessage(LOG, "Connected to WiFi.");
}

void startServer(int port, bool securityFlag) {
    // Find and Mount SPIFFS
    if (!SPIFFS.begin(true)) {
        logMessage(LOG, "Failed to mount SPIFFS.");
        return;
    }

    // Check if secuirty should be enabled
    if (securityFlag) {
        // Load server certificate and key in DER format from SPIFFS
        size_t certSize, keySize;
        unsigned char* serverCert = readBinaryFileFromSPIFFS(LOG, "/server_cert.der", &certSize);
        unsigned char* serverKey = readBinaryFileFromSPIFFS(LOG, "/server_key.der", &keySize);

        if (!serverCert || !serverKey) {
            logMessage(LOG, "Error loading certificates or private key.");
            return;
        }

        // Create SSL certificate object using DER format
        SSLCert *cert = new SSLCert(serverCert, certSize, serverKey, keySize);

        // Create HTTPS server using the SSL certificate
        serverHTTPS = new HTTPSServer(cert, port);
        logMessage(LOG, "Secure server init complete.");

        // Define a resource for the root path
        ResourceNode *node = new ResourceNode("/", "POST", &handleClientRequest);
        serverHTTP->registerNode(node);

        // Start the server
        serverHTTP->start();
        logMessage(LOG, "Authenticated Server started at port 443 in the root path.");

        // Erase the memory after creating the certificate
        delete[] serverCert;
        delete[] serverKey;

    } else {
        // Non-secure server: do not load certificates
        serverHTTP = new HTTPServer(port);
        logMessage(LOG, "Non-secure server init complete.");

        // Define a resource for the root path
        ResourceNode *node = new ResourceNode("/", "POST", &handleClientRequest);
        serverHTTP->registerNode(node);

        // Start the server
        serverHTTP->start();
        logMessage(LOG, "Unauthenticated Server started at port 80 in the root path.");
    }
}

/* ********************************************************************************************* */
