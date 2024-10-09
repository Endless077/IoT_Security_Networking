#ifndef WIFI_CLIENT_H
#define WIFI_CLIENT_H

#include <Arduino.h>

void setupWiFi(const char* ssid, const char* password);
void secureConnection(const char* serverAddress, int serverPort, const char* clientKey);
void notSecureConnection(const char* serverAddress, int serverPort, const char* clientKey);

#endif
