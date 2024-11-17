// Espressif - ESP32 Client (server.cpp)
#include <WiFi.h>

// Setting up the server
#include <HTTPServer.hpp>
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
        logMessage(LOG, "Not secure connection detected, be careful.");
    } else {
        // Secure server connection detected
        logMessage(LOG, "Secure connection detected, you are protected.");
    }

    // Init size and content of the request
    size_t contentLength = 0;
    String payload = "";
    
    // Print all Metadata available
    requestMetadata(contentLength, payload, req, res);

    // Check if the payload type is "text/plain"
    if (req->getHeader("Content-Type") != "text/plain") {
        // Logging invalid request payload (unsupported) error
        logMessage(LOG, "Bad Request: Invalid Request Payload (unsupported).");

        // Close the connection
        res->setStatusCode(415);
        res->println("Bad Request: invalid request payload (unsupported).");
        res->setHeader("Connection", "close");
        res->finalize();
        return;
    }

    // Check the payload request is not empty
    if (contentLength <= 0) {
        // Logging invalid request payload (empty) error
        logMessage(LOG, "Invalid Request Payload (empty).");
        setLedStatus(redLED, HIGH);

        // Close the connection
        res->setStatusCode(411);
        res->println("Bad Request: invalid request payload (empty).");
        res->setHeader("Connection", "close");
        res->finalize();
        return;
    }
    
    // Check the payload length valid
    if (payload.length() != contentLength) {
        // Logging invalid request payload (mismatch) error
        logMessage(LOG, "Invalid Request Payload (mismatch).");
        setLedStatus(redLED, HIGH);

        // Close the connection
        res->setStatusCode(400);
        res->println("Bad Request: invalid request payload (mismatch).");
        res->setHeader("Connection", "close");
        res->finalize();
        return;
    }

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
    if (payload == storedKey) {
        logMessage(LOG, "The Client key matches.");
        res->setStatusCode(200);
        res->println("The client key and the stored key matches (200).");
        setLedStatus(greenLED, HIGH);
    } else {
        logMessage(LOG, "The Client key does not match.");
        res->setStatusCode(401);
        res->println("The client key and the stored key does not match (401).");
        setLedStatus(redLED, HIGH);
    }
}

/* ********************************************************************************************* */

void shutdown() {
    // Logging the shutdown
    logMessage(LOG, "Shutting down server...");

    // Stop the HTTP Server
    if (serverHTTP != nullptr) {
        serverHTTP->stop();
        delete serverHTTP;
        serverHTTP = nullptr;
        logMessage(LOG, "HTTP server stopped.");
    }
    
    // Stop the HTTP Server
    if (serverHTTPS != nullptr) {
        serverHTTPS->stop();
        delete serverHTTPS;
        serverHTTPS = nullptr;
        logMessage(LOG, "HTTPS server stopped.");
    }
    
    // Reset the Service
    resetService();
    
    // WiFi Disconnect
    WiFi.disconnect(true, true);

    // SPIFFS Service Stop
    SPIFFS.end();

    // mDNS Service Stop
    MDNS.end();
    
    logMessage(LOG, "Server shutdown complete.");
}

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
        serverHTTPS->setDefaultNode(node404);

        // Start the server
        serverHTTPS->start();
        logMessage(LOG, (String("Authenticated Server started at port ") + port + " in the root path.").c_str());
    } else {
        // Non-secure server: do not load certificates
        serverHTTP = new HTTPServer(port);
        logMessage(LOG, "Non-secure server init complete.");

        // Define a resource for the root path
        ResourceNode * nodeRoot = new ResourceNode("/", "POST", &handleRequest);
        ResourceNode * node404  = new ResourceNode("", "POST", &handle404);

        serverHTTP->registerNode(nodeRoot);
        serverHTTP->setDefaultNode(node404);

        // Start the server
        serverHTTP->start();
        logMessage(LOG, (String("Unauthenticated Server started at port ") + port + " in the root path.").c_str());
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
