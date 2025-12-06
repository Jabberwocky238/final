#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "mbed.h"
#include "config.h"
#include "detector.h"

class BLEService {
private:
    UnbufferedSerial* bleSerial;
    bool deviceConnected;
    Timer connectionTimer;
    
    void sendCommand(const char* cmd);
    bool waitForResponse(const char* expected, uint32_t timeout_ms);
    
public:
    BLEService();
    ~BLEService();
    bool begin();
    void updateData(DetectionResult result);
    bool isConnected();
    void sendString(const char* str);
};

#endif