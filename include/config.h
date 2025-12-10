#ifndef CONFIG_H
#define CONFIG_H

// 采样配置
#define SAMPLE_RATE 52              // 采样率 52Hz
#define WINDOW_SIZE 128             // 2.46秒数据 (128样本, 必须是2的幂次方用于FFT)
#define SAMPLE_PERIOD_MS 19         // 1000/52 ≈ 19ms

// 频率范围定义
#define TREMOR_FREQ_MIN 3.0f        // 震颤最低频率 3Hz
#define TREMOR_FREQ_MAX 6.0f        // 震颤范围扩大到 6Hz
#define DYSKINESIA_FREQ_MIN 0.5f    // 运动障碍通常是低频不自主运动 (0.5-4Hz)
#define DYSKINESIA_FREQ_MAX 4.0f    // 

// 检测阈值 (降低以提高灵敏度)
#define TREMOR_THRESHOLD 0.02f      // 进一步降低阈值
#define DYSKINESIA_THRESHOLD 0.02f  // 进一步降低阈值
#define MOTION_THRESHOLD 0.30f      // 运动检测阈值 (降低以便检测停止)
#define FREEZE_TIME_MS 1500         // 冻结检测时间 1.5秒 (缩短)

// HM-10 BLE 模块配置
// 改用 USART2: TX=PA2, RX=PA3 (避免与 USBTX/USBRX 冲突)
// 注意：需要将 HM-10 模块连接到这些新引脚
#define BLE_TX PA_2
#define BLE_RX PA_3
#define BLE_BAUD 9600

// LSM6DSL I2C地址（板载传感器）
#define LSM6DSL_ADDR (0x6A << 1)    // mbed 使用 8-bit 地址

// I2C 引脚（板载传感器）- 避免与mbed定义冲突
#define SENSOR_I2C_SDA PB_11
#define SENSOR_I2C_SCL PB_10

#endif