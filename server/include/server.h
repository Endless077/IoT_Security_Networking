#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <Arduino.h>

void setupWiFi(const char* ssid, const char* password);
void startSecureServer(int port);
void startNonSecureServer(int port);

#endif
