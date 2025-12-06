#include "sensor.h"
#include <cmath>

SensorManager::SensorManager() {
    i2c = new I2C(SENSOR_I2C_SDA, SENSOR_I2C_SCL);
    i2c->frequency(400000); // 400kHz
    bufferIndex = 0;
    bufferFull = false;
    sampleReady = false;
}

SensorManager::~SensorManager() {
    stopSampling();
    delete i2c;
}

bool SensorManager::writeReg(uint8_t reg, uint8_t value) {
    char data[2] = {(char)reg, (char)value};
    int result = i2c->write(LSM6DSL_ADDR, data, 2);
    return (result == 0);
}

bool SensorManager::readRegs(uint8_t reg, uint8_t* data, int len) {
    char reg_addr = (char)reg;
    if (i2c->write(LSM6DSL_ADDR, &reg_addr, 1, true) != 0) {
        return false;
    }
    int result = i2c->read(LSM6DSL_ADDR, (char*)data, len);
    return (result == 0);
}

bool SensorManager::begin() {
    printf("Initializing LSM6DSL sensor...\r\n");
    
    // 检查 WHO_AM_I
    uint8_t whoami;
    if (!readRegs(WHO_AM_I_REG, &whoami, 1)) {
        printf("Failed to read WHO_AM_I\r\n");
        return false;
    }
    
    printf("WHO_AM_I: 0x%02X (expected 0x%02X)\r\n", whoami, EXPECTED_WHO_AM_I);
    
    if (whoami != EXPECTED_WHO_AM_I) {
        printf("Wrong sensor ID!\r\n");
        return false;
    }
    
    // 配置加速度计: 52Hz, ±2g
    // CTRL1_XL: ODR=52Hz (0011b), FS=±2g (00b), BW=400Hz (00b)
    if (!writeReg(CTRL1_XL, 0x30)) {  // 0011 0000
        printf("Failed to configure accelerometer\r\n");
        return false;
    }
    
    thread_sleep_for(100); // 等待传感器稳定
    
    printf("LSM6DSL initialized successfully\r\n");
    return true;
}

void SensorManager::sampleISR() {
    // ISR 中不能使用 I2C（需要互斥锁）
    // 只设置标志，数据读取在 update() 中进行
    sampleReady = true;
}

void SensorManager::startSampling() {
    printf("Starting sampling at 52Hz...\r\n");
    bufferIndex = 0;
    bufferFull = false;
    // 19ms = 19000us
    sampler.attach(callback(this, &SensorManager::sampleISR), 
                   std::chrono::microseconds(SAMPLE_PERIOD_MS * 1000));
}

void SensorManager::stopSampling() {
    sampler.detach();
}

void SensorManager::update() {
    // 检查是否需要采样
    if (!sampleReady) {
        return;
    }
    sampleReady = false;

    // 读取加速度计数据
    uint8_t data[6];
    if (!readRegs(OUTX_L_XL, data, 6)) {
        return;
    }

    // 转换为 int16_t
    int16_t ax_raw = (int16_t)((data[1] << 8) | data[0]);
    int16_t ay_raw = (int16_t)((data[3] << 8) | data[2]);
    int16_t az_raw = (int16_t)((data[5] << 8) | data[4]);

    // 转换为 m/s² (±2g, 灵敏度 0.061 mg/LSB)
    float ax = ax_raw * 0.061f / 1000.0f * 9.81f;
    float ay = ay_raw * 0.061f / 1000.0f * 9.81f;
    float az = az_raw * 0.061f / 1000.0f * 9.81f;

    // 减去重力
    az -= 9.81f;

    // 计算合成加速度
    float magnitude = sqrtf(ax*ax + ay*ay + az*az);

    // 存入缓冲区
    dataBuffer[bufferIndex] = magnitude;
    bufferIndex++;

    if (bufferIndex >= WINDOW_SIZE) {
        bufferIndex = 0;
        bufferFull = true;
    }
}

bool SensorManager::isBufferReady() {
    return bufferFull;
}

float* SensorManager::getDataBuffer() {
    return dataBuffer;
}

void SensorManager::clearBuffer() {
    bufferFull = false;
}

float SensorManager::getCurrentMagnitude() {
    uint8_t data[6];
    if (!readRegs(OUTX_L_XL, data, 6)) {
        return 0.0f;
    }
    
    int16_t ax_raw = (int16_t)((data[1] << 8) | data[0]);
    int16_t ay_raw = (int16_t)((data[3] << 8) | data[2]);
    int16_t az_raw = (int16_t)((data[5] << 8) | data[4]);
    
    float ax = ax_raw * 0.061f / 1000.0f * 9.81f;
    float ay = ay_raw * 0.061f / 1000.0f * 9.81f;
    float az = az_raw * 0.061f / 1000.0f * 9.81f - 9.81f;
    
    return sqrtf(ax*ax + ay*ay + az*az);
}