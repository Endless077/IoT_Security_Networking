#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

struct HttpRequest {
    int port;
    bool useHTTPS;
    const char* uri;
    const char* host;
    const char* method;
    const char* payload;
    const char* connection;
    const char* contentType;
    const char* userAgent;
};

/* ********************************************************************************************* */

// Logging Function
void logMessage(const char* tag, const char* message);

// Support SPIFFS (File System) Functions
String readFileFromSPIFFS(const char* path);
unsigned char* readBinaryFileFromSPIFFS(const char* path, uint16_t &length);

/* ********************************************************************************************* */

#endif
