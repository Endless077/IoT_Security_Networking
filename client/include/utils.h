#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

struct HttpRequest {
    int port;
    const char* host;
    const char* method;
    const char* path;
    const char* contentType;
    const char* body;
};

/* ********************************************************************************************* */

// Logging Function
void logMessage(const char* tag, const char* message);

// Support SPIFFS (File System) Functions
String readFileFromSPIFFS(const char* path);
unsigned char* readBinaryFileFromSPIFFS(const char* path, uint16_t &length);

/* ********************************************************************************************* */

#endif
