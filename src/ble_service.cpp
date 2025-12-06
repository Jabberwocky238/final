#include "ble_service.h"
#include <cstring>

BLEService::BLEService() {
    bleSerial = new UnbufferedSerial(BLE_TX, BLE_RX, BLE_BAUD);
    deviceConnected = false;
    connectionTimer.start();
}

BLEService::~BLEService() {
    delete bleSerial;
}

void BLEService::sendCommand(const char* cmd) {
    bleSerial->write(cmd, strlen(cmd));
    thread_sleep_for(100);
}

bool BLEService::waitForResponse(const char* expected, uint32_t timeout_ms) {
    char buffer[64];
    int idx = 0;
    uint32_t start = connectionTimer.elapsed_time().count() / 1000;
    
    while ((connectionTimer.elapsed_time().count() / 1000 - start) < timeout_ms) {
        char c;
        if (bleSerial->read(&c, 1) == 1) {
            buffer[idx++] = c;
            if (idx >= sizeof(buffer) - 1) break;
            buffer[idx] = '\0';
            
            if (strstr(buffer, expected) != NULL) {
                return true;
            }
        }
        thread_sleep_for(10);
    }
    return false;
}

bool BLEService::begin() {
    printf("Initializing HM-10 BLE module...\r\n");
    
    thread_sleep_for(1000); // 等待模块启动
    
    // 测试 AT 命令
    sendCommand("AT");
    if (!waitForResponse("OK", 1000)) {
        printf("Warning: No response from BLE module\r\n");
        printf("Check connections: TX=%d, RX=%d\r\n", BLE_TX, BLE_RX);
    } else {
        printf("BLE module responding\r\n");
    }
    
    // 设置设备名称
    sendCommand("AT+NAMEPDMonitor");
    thread_sleep_for(500);
    
    // 设置为从机模式
    sendCommand("AT+ROLE0");
    thread_sleep_for(500);
    
    // 复位模块使设置生效
    sendCommand("AT+RESET");
    thread_sleep_for(1000);
    
    printf("BLE Service initialized\r\n");
    printf("Device name: PDMonitor\r\n");
    printf("Waiting for connection...\r\n");
    
    deviceConnected = true; // 简化：假设连接
    return true;
}

void BLEService::updateData(DetectionResult result) {
    if (!deviceConnected) {
        return;
    }
    
    // 发送震颤数据
    char tremorData[64];
    snprintf(tremorData, sizeof(tremorData), "TREMOR:%d,%.2f\r\n", 
             result.tremorDetected ? 1 : 0, 
             result.tremorIntensity);
    sendString(tremorData);
    
    // 发送运动障碍数据
    char dyskData[64];
    snprintf(dyskData, sizeof(dyskData), "DYSK:%d,%.2f\r\n", 
             result.dyskinesiaDetected ? 1 : 0, 
             result.dyskinesiaIntensity);
    sendString(dyskData);
    
    // 发送冻结步态数据
    const char* stateStr;
    switch (result.motionState) {
        case MOTION_IDLE: stateStr = "IDLE"; break;
        case MOTION_WALKING: stateStr = "WALKING"; break;
        case MOTION_FROZEN: stateStr = "FROZEN"; break;
        default: stateStr = "UNKNOWN"; break;
    }
    
    char fogData[64];
    snprintf(fogData, sizeof(fogData), "FOG:%d,%s\r\n", 
             result.fogDetected ? 1 : 0, 
             stateStr);
    sendString(fogData);
    
    printf("BLE TX: %s%s%s", tremorData, dyskData, fogData);
}

bool BLEService::isConnected() {
    return deviceConnected;
}

void BLEService::sendString(const char* str) {
    bleSerial->write(str, strlen(str));
}