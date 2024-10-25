#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include "utils.h"

namespace httpsserver {
    class HTTPServer;
    class HTTPSServer;
}

extern httpsserver::HTTPServer *serverHTTP;
extern httpsserver::HTTPSServer *serverHTTPS;

/* ********************************************************************************************* */

void startServer(int port, bool useSecureConnection);
void setupWiFi(const char* ssid, const char* password);

/* ********************************************************************************************* */

#endif