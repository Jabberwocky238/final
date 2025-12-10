#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "detector.h"

// 使用 16-bit UUID 整数定义
const uint16_t PD_SERVICE_UUID = 0xA000;
const uint16_t TREMOR_CHAR_UUID = 0xA001;
const uint16_t DYSKINESIA_CHAR_UUID = 0xA002;
const uint16_t FOG_CHAR_UUID = 0xA003;

class BLEService : public ble::Gap::EventHandler {
private:
    BLE &_ble;
    events::EventQueue &_event_queue;
    Thread _event_thread;
    
    bool _connected;
    
    // 特征值句柄
    GattCharacteristic *_tremorChar;
    GattCharacteristic *_dyskinesiaChar;
    GattCharacteristic *_fogChar;
    
    // 数据缓冲区
    uint8_t _tremorValue[8];      // detected(1) + intensity(4)
    uint8_t _dyskinesiaValue[8];  // detected(1) + intensity(4)
    uint8_t _fogValue[8];         // detected(1) + state(1)
    
    // 广播数据缓冲区
    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::advertising_handle_t _adv_handle;

    void onInitComplete(BLE::InitializationCompleteCallbackContext *params);
    void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext *context);
    void startAdvertising();

    // Gap::EventHandler 回调重写
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;
    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;
    
public:
    BLEService();
    ~BLEService();
    
    void begin();
    void updateData(DetectionResult result);
    bool isConnected();
};

#endif
