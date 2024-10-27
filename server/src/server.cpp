// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>

// Setting up the server
#include <HTTPSServer.hpp>

// Define the certificate data for the server
#include <SSLCert.hpp>

// Define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required for ResourceNodes definition
#include <ResourceNode.hpp>

// SPIFFS Manager
#include <SPIFFS.h>

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

// Error 404 Handle Request
void handle404(HTTPRequest * req, HTTPResponse * res) {
  req->discardRequestBody();
  res->setStatusCode(404);
  res->setStatusText("Not Found");
  res->setHeader("Content-Type", "text/html");
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>Error 404 - Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}

// Service Handle Request
void handleRequest(HTTPRequest *req, HTTPResponse *res) {
    // Check if the connection is secure
    if (serverHTTPS != nullptr) {
        if (!req->isSecure()) {
            // Turn on the red LED for a not-secure or suspicious connection
            logMessage(LOG, "Not-secure connection detected, alarm started.");
            setLedStatus(redLED, true);
            return;
        } else {
            // Turn off the red LED if the connection is secure
            setLedStatus(redLED, false);
        }
    } else {
        // Log message indicating a HTTP Unsafe connection
        logMessage(LOG, "Not-secure connection started via HTTP.");
    }

    // Init size and content of the request
    size_t contentLength = 0;
    String bodyContent = "";

    // Print all Metadata available
    requestMetadata(contentLength, bodyContent, req, res);

    // Check if there is any request body
    if (contentLength > 0) {
        // Load the stored key from SPIFFS
        String storedKey = readFileFromSPIFFS("/secret.txt");

        // Check if the stored key is correctly uploaded
        if (storedKey.isEmpty()) {
            logMessage(LOG, "Failed to open secret.txt");
            res->setStatusCode(500);
            res->println("Error reading server key");
            setLedStatus(redLED, true);
            return;
        }
        
        // Compare the client key with the stored key
        if (bodyContent == storedKey) {
            logMessage(LOG, "Client key matches.");
            res->setStatusCode(200);
            res->println("Success, the client key and the stored key matches.");
            setLedStatus(greenLED, true);
        } else {
            logMessage(LOG, "Client key does not match.");
            res->setStatusCode(401);
            res->println("Failure, the client key and the stored key does not match.");
            setLedStatus(redLED, true);
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
        unsigned char* serverCert = readBinaryFileFromSPIFFS("/server_cert.der", &certSize);
        unsigned char* serverKey = readBinaryFileFromSPIFFS("/server_key.der", &keySize);

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
        ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRequest);
        ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

        serverHTTPS->registerNode(nodeRoot);
        serverHTTPS->registerNode(node404);

        // Start the server
        serverHTTPS->start();
        logMessage(LOG, "Authenticated Server started at port 443 in the root path.");

        // Erase the memory after creating the certificate
        delete[] serverCert;
        delete[] serverKey;

    } else {
        // Non-secure server: do not load certificates
        serverHTTP = new HTTPServer(port);
        logMessage(LOG, "Non-secure server init complete.");

        // Define a resource for the root path
        ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRequest);
        ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

        serverHTTPS->registerNode(nodeRoot);
        serverHTTPS->registerNode(node404);

        // Start the server
        serverHTTP->start();
        logMessage(LOG, "Unauthenticated Server started at port 80 in the root path.");
    }
}

/* ********************************************************************************************* */
