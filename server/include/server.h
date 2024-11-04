#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include "utils.h"

namespace httpsserver {
    class HTTPServer;
    class HTTPSServer;
}

extern httpsserver::HTTPServer *serverHTTP;
extern httpsserver::HTTPSServer *serverHTTPS;

/* ********************************************************************************************* */

// Handle Error Request Functions
void handleHandshake(int status, const char* msg);
void handle404(httpsserver::HTTPRequest * req, httpsserver::HTTPResponse * res);

//Handle Request Function
void handleRequest(httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);

// Main Functions
void shutdown();
void startServer(int port, bool useSecureConnection);
void setupWiFi(const char* ssid, const char* password);

/* ********************************************************************************************* */

#endif