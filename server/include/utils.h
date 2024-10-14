#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Settings Global Variables
extern const int redLED;
extern const int greenLED;

extern int currentLED;
extern unsigned long ledTimer;
extern const unsigned long ledDuration;

/* ********************************************************************************************* */

// Logging Function
void logMessage(const char* tag, const char* message);

// Reset Current Service
void resetService();

// Support SPIFFS (File System) Functions
String readFileFromSPIFFS(const char* tag, const char* path);
unsigned char* readBinaryFileFromSPIFFS(const char* tag, const char* path, size_t* fileSize);

/* ********************************************************************************************* */

#endif
