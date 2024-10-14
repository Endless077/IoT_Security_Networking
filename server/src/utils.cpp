// Utils
#include "utils.h"

// Other functions
#include <time.h>
#include <SPIFFS.h> 

// Settings Global Variables
const int redLED = 4;
const int greenLED = 13;

int currentLED = -1;
unsigned long ledTimer = 0;
const unsigned long ledDuration = 2000;

/* ********************************************************************************************* */

// Logging Function
void logMessage(const char* tag, const char* message) {
    // Local current date and time
    time_t now;
    time(&now);
    struct tm* timeInfo = localtime(&now);

    // Print the message with the requested format
    char timeStamp[32];
    strftime(timeStamp, sizeof(timeStamp), "%d/%m/%Y %H:%M:%S", timeInfo);
    Serial.printf("[%s - %s] %s\n", tag, timeStamp, message);
}

// Reset the current service
void resetService() {
    // Check if the LED needs to be turned off
    if (currentLED != -1 && millis() - ledTimer >= ledDuration) {
        // Turn off the LED that was on
        digitalWrite(currentLED, LOW);
        // Reset the current LED indicator
        currentLED = -1;
    }
}

// Support SPIFFS Functions
String readFileFromSPIFFS(const char* tag, const char* path) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        logMessage(tag, (String("Failed to open file: ") + path).c_str());
        return String();
    }
    String content = file.readString();
    file.close();
    return content;
}

unsigned char* readBinaryFileFromSPIFFS(const char* tag, const char* path, size_t* fileSize) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        logMessage(tag, (String("Failed to open file: ") + path).c_str());
        return nullptr;
    }
    *fileSize = file.size();
    unsigned char* buffer = new unsigned char[*fileSize];
    file.read(buffer, *fileSize);
    file.close();
    return buffer;
}

/* ********************************************************************************************* */
