#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <HTTPSServer.hpp>
#include <Arduino.h>

extern httpsserver::HTTPServer *serverHTTP;
extern httpsserver::HTTPSServer *serverHTTPS;

/* ********************************************************************************************* */

void startServer(int port, bool useSecureConnection);
void setupWiFi(const char* ssid, const char* password);

/* ********************************************************************************************* */

#endif