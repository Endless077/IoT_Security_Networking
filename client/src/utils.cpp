// Utils
#include "utils.h"
#include <time.h>

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
