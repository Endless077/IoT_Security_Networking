#ifndef UTILS_H
#define UTILS_H

// Include Arduino Framework Core
#include <Arduino.h>

// HTTP Request and Response
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

// Settings Global Variables
extern const int redLED;
extern const int greenLED;

extern unsigned long ledTimer;
extern const unsigned long ledDuration;

/* ********************************************************************************************* */

// Logging Function
void logMessage(const char* tag, const char* message);

// Setting Led Status
void setLedStatus(int ledPin, int state);

// Reset Current Service
void resetService();

// Request Metadata
void requestMetadata(size_t &contentLength, String &bodyContent, httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res);

// Support SPIFFS (File System) Functions
String readFileFromSPIFFS(const char* path);
unsigned char* readBinaryFileFromSPIFFS(const char* path, size_t* fileSize);

/* ********************************************************************************************* */

#endif
