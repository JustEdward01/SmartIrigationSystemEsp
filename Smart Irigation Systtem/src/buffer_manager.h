#pragma once
#include <Arduino.h>
class BufferManager {
public:
    BufferManager();
    void init();
    void save(const String&);
    void flushToServer(bool(*sendFn)(const String&));
    int countLines();
    void enforceLimit();
    bool hasSpace();
    void backupBuffer();
    void restoreBuffer();
};
