#ifndef WIFI_CLIENT_H
#define WIFI_CLIENT_H

#include "utils.h"

/* ********************************************************************************************* */

// Main Functions
void secureConnection(const HttpRequest& request);
void notSecureConnection(const HttpRequest& request);
void setupWiFi(const char* ssid, const char* password);

/* ********************************************************************************************* */

#endif
