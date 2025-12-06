#include "mbed.h"
#include "config.h"
#include "sensor.h"
#include "detector.h"
// #include "ble_service.h"  // 暂时禁用 BLE

// 重定向 stdout 到硬件串口 (修复串口输出问题)
UnbufferedSerial pc(USBTX, USBRX, 115200);

namespace mbed {
    FileHandle *mbed_override_console(int fd) {
        return &pc;
    }
}

// 全局对象
SensorManager sensor;
Detector detector;
// BLEService bleService;  // 暂时禁用 BLE

// LED
DigitalOut led1(LED1);

// 状态
DetectionResult currentResult;

int main() {
    // 等待串口稳定
    ThisThread::sleep_for(1000ms);

    printf("\r\n\r\n\r\nSTART\r\n");  // 简单测试
    printf("\r\n=================================\r\n");
    printf("Parkinson's Disease Monitor\r\n");
    printf("STM32L475 Discovery Kit IoT\r\n");
    printf("=================================\r\n\r\n");
    
    // 初始化传感器
    printf("Initializing sensor...\r\n");
    if (!sensor.begin()) {
        printf("ERROR: Sensor initialization failed!\r\n");
        while(1) {
            led1 = !led1;
            thread_sleep_for(100);
        }
    }
    
    // 初始化 BLE (暂时禁用)
    // printf("Initializing BLE...\r\n");
    // bleService.begin();

    // 启动采样
    sensor.startSampling();
    
    // 状态 LED 常亮
    led1 = 1;
    
    printf("\r\nSystem ready!\r\n");
    printf("Waiting for data...\r\n\r\n");
    
    while (1) {
        // 更新传感器（读取数据）
        sensor.update();

        // 检查是否有足够数据
        if (sensor.isBufferReady()) {
            printf("\r\n--- Data ready ---\r\n");

            // 获取数据缓冲区
            float* buffer = sensor.getDataBuffer();

            // 打印前几个样本
            printf("First samples: ");
            for (int i = 0; i < 5 && i < WINDOW_SIZE; i++) {
                printf("%.2f ", buffer[i]);
            }
            printf("\r\n");

            // 获取当前运动强度 (用于 FOG 检测)
            float currentMotion = sensor.getCurrentMagnitude();

            // 运行检测器分析
            printf("Running detector analysis...\r\n");
            currentResult = detector.analyze(sensor.getDataBuffer(), currentMotion);

            // 更新 BLE 数据 (暂时禁用)
            // bleService.updateData(currentResult);

            // 清除缓冲区标志
            sensor.clearBuffer();
            
            // LED 指示
            if (currentResult.tremorDetected || 
                currentResult.dyskinesiaDetected || 
                currentResult.fogDetected) {
                led1 = !led1;  // 检测到异常时闪烁
            } else {
                led1 = 1;  // 正常时常亮
            }
            
            // 打印结果
            printf("\r\n--- Detection Summary ---\r\n");
            printf("Tremor: %s (Intensity: %.2f)\r\n", 
                   currentResult.tremorDetected ? "YES" : "NO", 
                   currentResult.tremorIntensity);
            printf("Dyskinesia: %s (Intensity: %.2f)\r\n", 
                   currentResult.dyskinesiaDetected ? "YES" : "NO", 
                   currentResult.dyskinesiaIntensity);
            printf("FOG: %s (State: %d)\r\n", 
                   currentResult.fogDetected ? "YES" : "NO", 
                   currentResult.motionState);
            printf("-------------------------\r\n\r\n");
        }
        
        thread_sleep_for(10);
    }
}