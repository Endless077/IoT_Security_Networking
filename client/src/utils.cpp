// Utils
#include "utils.h"

// Other functions
#include <time.h>
#include <SPIFFS.h>

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

// Support SPIFFS Functions
String readFileFromSPIFFS(const char* path) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        logMessage("SPIFFS", (String("Failed to open file: ") + path).c_str());
        return String();
    }
    String content = file.readString();
    file.close();
    return content;
}

unsigned char* readBinaryFileFromSPIFFS(const char* path, size_t* fileSize) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        logMessage("SPIFFS", (String("Failed to open file: ") + path).c_str());
        return nullptr;
    }
    *fileSize = file.size();
    unsigned char* buffer = new unsigned char[*fileSize];
    file.read(buffer, *fileSize);
    file.close();
    return buffer;
}

/* ********************************************************************************************* */
