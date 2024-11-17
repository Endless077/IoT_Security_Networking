// Espressif - ESP32 Client (client.cpp)
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// mDNS Manager
#include <ESPmDNS.h>

// SPIFFS Manager
#include <SPIFFS.h>

// Own libraries
#include "client.h"
#include "utils.h"

/* ********************************************************************************************* */

const char* LOG = "Client";

/* ********************************************************************************************* */

void shutdown() {
    // Logging the shutdown
    logMessage(LOG, "Shutting down client...");
    
    // WiFi Disconnect
    WiFi.disconnect(true, true);

    // SPIFFS End
    SPIFFS.end();
    
    // mDNS End
    MDNS.end();
    
    logMessage(LOG, "Client shutdown complete.");
}

void sendRequest(const HttpRequest& request) {
    // Init the HTTPClient instance
    HTTPClient http;
    String url;

    // Payload check
    if (request.payload == nullptr || strlen(request.payload) == 0) {
        logMessage(LOG, "Request body is empty or null. Request aborted.");
        return;
    }

    if (request.useHTTPS) {
        // Init a WiFiClientSecure instance (secure)
        WiFiClientSecure secureClient;
        
        // Retrive the HTTPS request URL
        url = String("https://") + request.host + ":" + request.port + request.uri;
        logMessage(LOG, (String("Trying connection to: ") + url).c_str());

        // Load the CA certificate manually from SPIFFS
        logMessage(LOG, "Loading CA Certificate...");
        String caCert = readFileFromSPIFFS("/ca_cert.pem");

        // Set the CA certificate
        if (caCert.length() > 0) {
            secureClient.setCACert(caCert.c_str());
            logMessage(LOG, "CA Certificate loaded.");
        }

        // Load the client certificate manually from SPIFFS
        logMessage(LOG, "Loading Client Certificate...");
        String clientCert = readFileFromSPIFFS("/client_cert.pem");

        // Set the client certificate
        if (clientCert.length() > 0) {
            secureClient.setCertificate(clientCert.c_str());
            logMessage(LOG, "Client Certificate loaded.");
        }
        
        // Load the client key manually from SPIFFS
        logMessage(LOG, "Loading Client Private Key...");
        String clientKey = readFileFromSPIFFS("/client_key.pem");

        // Set the client private key
        if (clientKey.length() > 0) {
            secureClient.setPrivateKey(clientKey.c_str());
            logMessage(LOG, "Client Private Key loaded.");
        }

        // Begin the HTTPS request (uncomment only one the type of configuration)
        
        //http.begin(url, caCert);
        //http.begin(secureClient, url);
        http.begin(request.host, request.port, request.uri, caCert.c_str());
        //http.begin(secureClient, request.host, request.port, request.uri, request.useHTTPS);
        //http.begin(request.host, request.port, request.uri, caCert.c_str(), clientCert.c_str(), clientKey.c_str());
    } else {
        // Init a WiFiClient instance (not secure)
        WiFiClient client;

        // Retrive the HTTP request URL
        url = String("http://") + request.host + ":" + request.port + request.uri;
        logMessage(LOG, (String("Trying connection to: ") + url).c_str());

        // Begin the HTTP request (uncomment only one the type of configuration)
        
        //http.begin(url);
        //http.begin(client, url);
        http.begin(request.host, request.port, request.uri);
        //http.begin(client, request.host, request.port, request.uri, request.useHTTPS);
    }

    // Setup the Headers
    http.addHeader("Host", request.host);
    http.addHeader("Connection", request.connection);
    http.addHeader("Content-Type", request.contentType);
    http.addHeader("Content-Length", String(strlen(request.payload)));

    // Setup TCP Timeout
    http.setTimeout(10000);
    
    // Setup User Agent
    http.setUserAgent(request.userAgent);

    // Start the HTTP Connection
    int status = http.POST(request.payload);
    String response = http.getString();
    response.trim(); 

    if (http.connected()) {
        logMessage(LOG, request.useHTTPS ? "Connection to secure server alive." : "Connection to not secure server alive.");

        // Response Management
        if (status >= 200 && status < 300) {
            logMessage(LOG, (String("Success response (" + String(status) + "): ") + response).c_str());
        } else if (status >= 300 && status < 400) {
            logMessage(LOG, (String("Warning Redirection response (" + String(status) + "): ") + response).c_str());
        } else if (status >= 400 && status < 500) {
            logMessage(LOG, (String("Error Client response (" + String(status) + "): ") + response).c_str());
        } else if (status >= 500 && status < 600) {
            logMessage(LOG, (String("Error Server response (" + String(status) + "): ") + response).c_str());
        } else {
            logMessage(LOG, (String("Unexpected status code: ") + String(status)).c_str());
        }
        
        // Close the connection
        http.end();
        logMessage(LOG, request.useHTTPS ? "HTTPS connection closed." : "HTTP connection closed."); 
    } else {
        logMessage(LOG, request.useHTTPS ? "Connection to secure server dead." : "Connection to not secure server dead.");
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
    if (!MDNS.begin("esp32client")) {
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
