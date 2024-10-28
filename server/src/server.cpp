// Espressif - ESP32 Client (server.cpp)
#include <WiFi.h>

// Setting up the server
#include <HTTPSServer.hpp>

// Define the certificate data for the server
#include <SSLCert.hpp>

// Define request handler callbacks
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Required for ResourceNodes definition
#include <ResourceNode.hpp>

// mDNS Manager
#include <ESPmDNS.h>

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

// Error SSL/TLS Handshake
void handleHandshake(int status, const char* msg) {
    // Logging the Handshake result and do something
    if (!status) setLedStatus(redLED, HIGH);
    logMessage(LOG, msg);
}

// HTTP Error Code: 404 (Not Found)
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
    // Check the server status connection
    if (!req->isSecure()) {
        // Not-secure or suspicious server connection detected
        logMessage(LOG, "Not-secure connection detected, be careful.");
    } else {
        // Secure server connection detected
        logMessage(LOG, "Secure connection detected, you are protected.");
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
            res->println("Error reading server key.");
            setLedStatus(redLED, HIGH);
            return;
        }
        
        // Compare the client key with the stored key
        if (bodyContent == storedKey) {
            logMessage(LOG, "Client key matches.");
            res->setStatusCode(200);
            res->println("Success, the client key and the stored key matches.");
            setLedStatus(greenLED, HIGH);
        } else {
            logMessage(LOG, "Client key does not match.");
            res->setStatusCode(401);
            res->println("Failure, the client key and the stored key does not match.");
            setLedStatus(redLED, HIGH);
        }
    } else {
        res->setStatusCode(400);
        res->println("Bad Request: no content.");
    }
}

/* ********************************************************************************************* */

void startServer(int port, bool securityFlag) {
    // Initialize SPIFFS (file system)
    if (!SPIFFS.begin(true)) {
        logMessage(LOG, "Failed to mount SPIFFS.");
        ESP.restart();
    }

    // Check if secuirty should be enabled
    if (securityFlag) {
        // Load server certificate and key in DER format from SPIFFS
        uint16_t certSize, keySize;
        
        unsigned char *certBuffer = readBinaryFileFromSPIFFS("/server_cert.der", certSize);
        unsigned char *keyBuffer = readBinaryFileFromSPIFFS("/server_key.der", keySize);

        if (certBuffer == nullptr || keyBuffer == nullptr) {
            logMessage(LOG, "Error loading certificates or private key.");
            ESP.restart();
        }

        // Create SSL certificate object using DER format
        SSLCert cert(certBuffer, certSize, keyBuffer, keySize);

        // Create HTTPS server using the SSL certificate
        serverHTTPS = new HTTPSServer(&cert, port);
        logMessage(LOG, "Secure server init complete.");

        // Define a resource for the root path
        ResourceNode * nodeRoot = new ResourceNode("/", "POST", &handleRequest);
        ResourceNode * node404  = new ResourceNode("", "POST", &handle404);

        serverHTTPS->registerNode(nodeRoot);
        serverHTTPS->registerNode(node404);

        // Start the server
        serverHTTPS->start();
        logMessage(LOG, "Authenticated Server started at port 443 in the root path.");

    } else {
        // Non-secure server: do not load certificates
        serverHTTP = new HTTPServer(port);
        logMessage(LOG, "Non-secure server init complete.");

        // Define a resource for the root path
        ResourceNode * nodeRoot = new ResourceNode("/", "POST", &handleRequest);
        ResourceNode * node404  = new ResourceNode("", "POST", &handle404);

        serverHTTP->registerNode(nodeRoot);
        serverHTTP->registerNode(node404);

        // Start the server
        serverHTTP->start();
        logMessage(LOG, "Unauthenticated Server started at port 80 in the root path.");
    }
}

void setupWiFi(const char* ssid, const char* password) {
    // Start a WiFi connection with given SSID and Password
    WiFi.begin(ssid, password);
    logMessage(LOG, "Connecting to WiFi...");
    
    // Start the WiFi connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        logMessage(LOG, ".");
    }

    // Get the IP Address of the server
    logMessage(LOG, (String("IP Address: ") + WiFi.localIP().toString()).c_str());
    logMessage(LOG, "Connected to WiFi.");

    // Start mDNS
    if (!MDNS.begin("esp32server")) {
        logMessage(LOG, "Error setting up MDNS responder.");
        ESP.restart();
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
