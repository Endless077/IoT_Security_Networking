#ifndef WIFI_CLIENT_H
#define WIFI_CLIENT_H

#include "utils.h"

/* ********************************************************************************************* */

// Main Functions
void shutdown();
void sendRequest(const HttpRequest& request);
void setupWiFi(const char* ssid, const char* password);

/* ********************************************************************************************* */

#endif
