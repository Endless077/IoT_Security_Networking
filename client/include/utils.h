#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Logging Function
void logMessage(const char* tag, const char* message);

// Support SPIFFS (File System) Functions
String readFileFromSPIFFS(const char* path);
unsigned char* readBinaryFileFromSPIFFS(const char* path, size_t* fileSize);

#endif
