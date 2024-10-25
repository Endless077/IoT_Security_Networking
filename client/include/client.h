#ifndef WIFI_CLIENT_H
#define WIFI_CLIENT_H

#include "utils.h"

/* ********************************************************************************************* */

void setupWiFi(const char* ssid, const char* password);
void secureConnection(const HttpRequest& request);
void notSecureConnection(const HttpRequest& request);

/* ********************************************************************************************* */

#endif
