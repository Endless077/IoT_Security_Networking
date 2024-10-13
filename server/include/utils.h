#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Logging Function
void logMessage(const char* tag, const char* message);

// Support SPIFFS Functions
String readFileFromSPIFFS(const char* tag, const char* path);
unsigned char* readBinaryFileFromSPIFFS(const char* tag, const char* path, size_t* fileSize);

#endif
