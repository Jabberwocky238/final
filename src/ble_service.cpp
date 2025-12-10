#include "ble_service.h"

// 静态事件队列，用于处理 BLE 事件
static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

BLEService::BLEService() : 
    _ble(BLE::Instance()), 
    _event_queue(event_queue),
    _connected(false),
    _tremorChar(nullptr),
    _dyskinesiaChar(nullptr),
    _fogChar(nullptr),
    _adv_handle(ble::LEGACY_ADVERTISING_HANDLE)
{
}

BLEService::~BLEService() {
    // 清理资源
}

void BLEService::begin() {
    printf("Initializing onboard BLE...\r\n");
    
    // 绑定事件处理回调
    _ble.onEventsToProcess(makeFunctionPointer(this, &BLEService::scheduleBleEventsProcessing));

    // 初始化 BLE
    _ble.init(this, &BLEService::onInitComplete);

    // 启动事件处理线程
    _event_thread.start(callback(&_event_queue, &events::EventQueue::dispatch_forever));
}

void BLEService::scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext *context) {
    _event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void BLEService::onInitComplete(BLE::InitializationCompleteCallbackContext *params) {
    if (params->error != BLE_ERROR_NONE) {
        printf("BLE initialization failed.\r\n");
        return;
    }

    printf("BLE initialized successfully.\r\n");
    
    // 设置 Gap 事件处理程序 (this 类实现了 Gap::EventHandler)
    _ble.gap().setEventHandler(this);

    // 配置特征值
    // 1. Tremor: Read + Notify
    UUID tremorUUID(TREMOR_CHAR_UUID);
    _tremorChar = new GattCharacteristic(tremorUUID, _tremorValue, sizeof(_tremorValue), sizeof(_tremorValue), 
                                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

    // 2. Dyskinesia: Read + Notify
    UUID dyskinesiaUUID(DYSKINESIA_CHAR_UUID);
    _dyskinesiaChar = new GattCharacteristic(dyskinesiaUUID, _dyskinesiaValue, sizeof(_dyskinesiaValue), sizeof(_dyskinesiaValue), 
                                             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

    // 3. FOG: Read + Notify
    UUID fogUUID(FOG_CHAR_UUID);
    _fogChar = new GattCharacteristic(fogUUID, _fogValue, sizeof(_fogValue), sizeof(_fogValue), 
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

    // 配置服务
    GattCharacteristic *charTable[] = { _tremorChar, _dyskinesiaChar, _fogChar };
    UUID pdServiceUUID(PD_SERVICE_UUID);
    GattService pdService(pdServiceUUID, charTable, 3);

    _ble.gattServer().addService(pdService);

    // 启动广播
    startAdvertising();
}

void BLEService::startAdvertising() {
    // 使用 AdvertisingDataBuilder 构建广播包
    ble::AdvertisingDataBuilder adv_data_builder(_adv_buffer);

    adv_data_builder.setFlags(); // 默认 flags
    adv_data_builder.setName("PDMonitor");
    
    // 添加 16-bit UUID
    UUID pdServiceUUID(PD_SERVICE_UUID);
    adv_data_builder.setLocalServiceList(mbed::make_Span(&pdServiceUUID, 1));

    // 设置广播参数
    ble::AdvertisingParameters adv_params(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(1000))
    );

    // 设置 payload 并启动
    ble_error_t error = _ble.gap().setAdvertisingParameters(_adv_handle, adv_params);
    if (error) {
        printf("Error setting advertising params: %d\r\n", error);
        return;
    }

    error = _ble.gap().setAdvertisingPayload(_adv_handle, adv_data_builder.getAdvertisingData());
    if (error) {
        printf("Error setting advertising payload: %d\r\n", error);
        return;
    }

    error = _ble.gap().startAdvertising(_adv_handle);
    if (error) {
        printf("Error starting advertising: %d\r\n", error);
        return;
    }

    printf("BLE advertising started. Name: PDMonitor\r\n");
}

void BLEService::onConnectionComplete(const ble::ConnectionCompleteEvent &event) {
    if (event.getStatus() == BLE_ERROR_NONE) {
        printf("Device connected!\r\n");
        _connected = true;
    }
}

void BLEService::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) {
    printf("Device disconnected. Restarting advertising...\r\n");
    _connected = false;
    startAdvertising();
}

void BLEService::updateData(DetectionResult result) {
    if (!_connected) return;

    // 更新 Tremor 数据
    _tremorValue[0] = result.tremorDetected ? 1 : 0;
    memcpy(&_tremorValue[1], &result.tremorIntensity, sizeof(float));
    _ble.gattServer().write(_tremorChar->getValueHandle(), _tremorValue, 5);

    // 更新 Dyskinesia 数据
    _dyskinesiaValue[0] = result.dyskinesiaDetected ? 1 : 0;
    memcpy(&_dyskinesiaValue[1], &result.dyskinesiaIntensity, sizeof(float));
    _ble.gattServer().write(_dyskinesiaChar->getValueHandle(), _dyskinesiaValue, 5);

    // 更新 FOG 数据
    _fogValue[0] = result.fogDetected ? 1 : 0;
    _fogValue[1] = (uint8_t)result.motionState;
    _ble.gattServer().write(_fogChar->getValueHandle(), _fogValue, 2);
}

bool BLEService::isConnected() {
    return _connected;
}
