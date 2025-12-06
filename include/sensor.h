#ifndef SENSOR_H
#define SENSOR_H

#include "mbed.h"
#include "config.h"

class SensorManager {
private:
    I2C* i2c;
    Ticker sampler;
    float dataBuffer[WINDOW_SIZE];
    int bufferIndex;
    volatile bool bufferFull;
    volatile bool sampleReady;  // ISR 设置的标志

    // LSM6DSL 寄存器
    static constexpr uint8_t WHO_AM_I_REG = 0x0F;
    static constexpr uint8_t CTRL1_XL = 0x10;
    static constexpr uint8_t OUTX_L_XL = 0x28;
    static constexpr uint8_t EXPECTED_WHO_AM_I = 0x6A;
    
    bool writeReg(uint8_t reg, uint8_t value);
    bool readRegs(uint8_t reg, uint8_t* data, int len);
    void sampleISR();
    
public:
    SensorManager();
    ~SensorManager();
    bool begin();
    void startSampling();
    void stopSampling();
    void update();  // 在主循环中调用，读取传感器数据
    bool isBufferReady();
    float* getDataBuffer();
    void clearBuffer();
    float getCurrentMagnitude();
};

#endif