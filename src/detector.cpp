#include "detector.h"
#include <cmath>

Detector::Detector() {
    lastTremorIntensity = 0;
    lastDyskinesiaIntensity = 0;
    currentState = MOTION_IDLE;
    lastMotionTime = 0;
    walkingStartTime = 0;
    motionHistoryIndex = 0;
    
    for (int i = 0; i < 3; i++) {
        motionHistory[i] = 0;
    }
    
    timer.start();
}

DetectionResult Detector::analyze(float* data, float currentMotion) {
    DetectionResult result;
    
    // FFT 分析
    FrequencyPeak peak = fftProcessor.process(data);
    
    printf("Peak: %.2f Hz, Magnitude: %.3f\r\n", peak.frequency, peak.magnitude);
    
    // 检测震颤
    result.tremorDetected = detectTremor(peak, &result.tremorIntensity);
    
    // 检测运动障碍
    result.dyskinesiaDetected = detectDyskinesia(peak, &result.dyskinesiaIntensity);
    
    // 更新运动状态
    updateMotionState(currentMotion);
    
    // 检测冻结步态
    result.fogDetected = detectFOG(currentMotion);
    result.motionState = currentState;
    
    return result;
}

bool Detector::detectTremor(FrequencyPeak peak, float* intensity) {
    if (peak.frequency >= TREMOR_FREQ_MIN && peak.frequency <= TREMOR_FREQ_MAX) {
        *intensity = peak.magnitude;
        lastTremorIntensity = 0.7f * peak.magnitude + 0.3f * lastTremorIntensity;
        
        if (lastTremorIntensity > TREMOR_THRESHOLD) {
            printf(">>> TREMOR DETECTED <<<\r\n");
            return true;
        }
    } else {
        lastTremorIntensity *= 0.8f;
    }
    
    *intensity = lastTremorIntensity;
    return false;
}

bool Detector::detectDyskinesia(FrequencyPeak peak, float* intensity) {
    if (peak.frequency >= DYSKINESIA_FREQ_MIN && peak.frequency <= DYSKINESIA_FREQ_MAX) {
        *intensity = peak.magnitude;
        lastDyskinesiaIntensity = 0.7f * peak.magnitude + 0.3f * lastDyskinesiaIntensity;
        
        if (lastDyskinesiaIntensity > DYSKINESIA_THRESHOLD) {
            printf(">>> DYSKINESIA DETECTED <<<\r\n");
            return true;
        }
    } else {
        lastDyskinesiaIntensity *= 0.8f;
    }
    
    *intensity = lastDyskinesiaIntensity;
    return false;
}

void Detector::updateMotionState(float currentMotion) {
    motionHistory[motionHistoryIndex] = currentMotion;
    motionHistoryIndex = (motionHistoryIndex + 1) % 3;

    float avgMotion = getAverageMotion();
    float variance = getMotionVariance();
    float stdDev = sqrtf(variance);  // 标准差
    uint32_t currentTime = timer.elapsed_time().count() / 1000; // ms

    // 调试信息: 使用标准差判断是否在运动
    // 标准差大 = 运动值波动大 = 走路
    // 标准差小 = 运动值稳定 = 静止
    printf("Motion: %.2f, Avg: %.2f, StdDev: %.3f\r\n",
           currentMotion, avgMotion, stdDev);

    // 使用标准差阈值判断是否在运动
    // 提高阈值到 0.2 (0.05太敏感，0.1有时很难触发，试取中间或偏高以防误触)
    bool isMoving = (stdDev > 0.2f);

    switch (currentState) {
        case MOTION_IDLE:
            if (isMoving) {
                currentState = MOTION_WALKING;
                walkingStartTime = currentTime;
                lastMotionTime = currentTime;
                printf("State: IDLE -> WALKING (stddev=%.3f)\r\n", stdDev);
            }
            break;

        case MOTION_WALKING:
            if (isMoving) {
                lastMotionTime = currentTime;
                // ...
            } else {
                uint32_t stopTime = currentTime - lastMotionTime;
                uint32_t walkTime = currentTime - walkingStartTime;
                
                // ...
                if (stopTime > FREEZE_TIME_MS) {
                    // 只要走过 1.5秒 (1500ms) 然后停下，就视为冻结
                    // 增加一点行走时间要求，避免碰一下就触发
                    if (walkTime > 1500) {
                        currentState = MOTION_FROZEN;
                        printf("State: WALKING -> FROZEN\r\n");
                        // 记录进入冻结的时间，用于超时复位（复用 lastMotionTime 或新变量，这里简单处理复用逻辑）
                        lastMotionTime = currentTime; 
                    } else {
                        currentState = MOTION_IDLE;
                        printf("State: WALKING -> IDLE (walk too short)\r\n");
                    }
                }
            }
            break;

        case MOTION_FROZEN:
            if (isMoving) {
                currentState = MOTION_WALKING;
                walkingStartTime = currentTime;
                lastMotionTime = currentTime;
                printf("State: FROZEN -> WALKING (resumed, stddev=%.3f)\r\n", stdDev);
            } else {
                // 如果在冻结状态下持续静止超过 5秒，自动复位到 IDLE
                // 这里利用 lastMotionTime (在进入 FROZEN 时更新过) 计算时间
                if (currentTime - lastMotionTime > 5000) {
                    currentState = MOTION_IDLE;
                    printf("State: FROZEN -> IDLE (timeout)\r\n");
                }
            }
            break;
    }
}

bool Detector::detectFOG(float currentMotion) {
    return (currentState == MOTION_FROZEN);
}

float Detector::getAverageMotion() {
    float sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += motionHistory[i];
    }
    return sum / 3.0f;
}

float Detector::getMotionVariance() {
    float avg = getAverageMotion();
    float variance = 0;
    for (int i = 0; i < 3; i++) {
        float diff = motionHistory[i] - avg;
        variance += diff * diff;
    }
    return variance / 3.0f;
}

void Detector::reset() {
    lastTremorIntensity = 0;
    lastDyskinesiaIntensity = 0;
    currentState = MOTION_IDLE;
    lastMotionTime = 0;
    walkingStartTime = 0;
}