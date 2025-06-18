#pragma once
#include <WebServer.h>
class DebugServer {
public:
    DebugServer(WebServer&);
    void setupEndpoints();
};
