// Utils
#include "utils.h"

// Other functions
#include <time.h>
#include <SPIFFS.h> 

// Settings Global Variables
const int redLED = 2;
const int greenLED = 13;

unsigned long ledTimer = 0;
const unsigned long ledDuration = 3000;

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

// Setting Led Status
void setLedStatus(int ledPin, int state) {
    if (state == HIGH) {
        digitalWrite(ledPin, state);
        ledTimer = millis();
    }else{
        digitalWrite(ledPin, state);
        ledTimer = 0;
    }
}

// Reset the current service
void resetService() {
    // Check if the LED needs to be turned off
    if (millis() - ledTimer >= ledDuration) {
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, LOW);
    }
}

// Request Metadata
void requestMetadata(size_t &contentLength, String &payload, httpsserver::HTTPRequest *req, httpsserver::HTTPResponse *res) {
    // Logging the Request Metadata
    logMessage("Metadata", "----- Request Metadata -----");

    // Print client IP address
    IPAddress clientIP = req->getClientIP();
    logMessage("Metadata", (String("Client IP: ") + clientIP.toString()).c_str());

    // Print the URL of the request
    String url = String(req->getRequestString().c_str());
    logMessage("Metadata", (String("Resource: ") + url).c_str());

    // Print the method of the request (GET, POST, etc.)
    String method = String(req->getMethod().c_str());
    logMessage("Metadata", (String("Method: ") + method).c_str());

    logMessage("Metadata", "---");

    // Print the headers of the request
    logMessage("Metadata", "Headers:");
    httpsserver::HTTPHeaders *headers = req->getHTTPHeaders();
    std::vector<httpsserver::HTTPHeader *> *headerList = headers->getAll();

    for (auto header : *headerList) {
        String message = "--" + String(header->_name.c_str()) + ": " + String(header->_value.c_str());
        logMessage("Metadata", message.c_str());
    }

    logMessage("Metadata", "---");
    
    // Set the content length of the request
    contentLength = req->getContentLength();
    logMessage("Metadata", (String("Content-Length: ") + String(contentLength)).c_str());

    // Log the body of the request if it exists
    if (contentLength > 0) {
        // Allocate buffer for reading the content
        char *buffer = new char[contentLength + 1];
        size_t bytesRead = req->readChars(buffer, contentLength);

        // Null-terminate the content to treat it as a string
        buffer[bytesRead] = '\0';

        // Set the body content
        payload = String(buffer);

        // Log the body content
        logMessage("Metadata", (String("Request Body: ") + payload).c_str());

        // Free the buffer
        delete[] buffer;
    } else {
        payload = "";
        logMessage("Metadata", "Error: no content.");
    }

    logMessage("Metadata", "----------------------------");
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

unsigned char* readBinaryFileFromSPIFFS(const char* path, uint16_t &length) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        logMessage("SPIFFS", (String("Failed to open file: ") + path).c_str());
        return nullptr;
    }
    
    length = file.size();
    unsigned char *buffer = new unsigned char[length];

    if (buffer == nullptr) {
        logMessage("SPIFFS", "Error during memory allocation.");
        file.close();
        return nullptr;
    }
    
    file.read(buffer, length);
    file.close();
    return buffer;
}

/* ********************************************************************************************* */
