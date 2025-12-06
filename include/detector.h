#ifndef DETECTOR_H
#define DETECTOR_H

#include "mbed.h"
#include "config.h"
#include "fft_processor.h"

enum MotionState {
    MOTION_IDLE,
    MOTION_WALKING,
    MOTION_FROZEN
};

struct DetectionResult {
    bool tremorDetected;
    float tremorIntensity;
    
    bool dyskinesiaDetected;
    float dyskinesiaIntensity;
    
    bool fogDetected;
    MotionState motionState;
};

class Detector {
private:
    FFTProcessor fftProcessor;
    
    float lastTremorIntensity;
    float lastDyskinesiaIntensity;
    
    MotionState currentState;
    Timer timer;
    uint32_t lastMotionTime;
    uint32_t walkingStartTime;
    float motionHistory[3];
    int motionHistoryIndex;
    
    bool detectTremor(FrequencyPeak peak, float* intensity);
    bool detectDyskinesia(FrequencyPeak peak, float* intensity);
    bool detectFOG(float currentMotion);
    void updateMotionState(float currentMotion);
    float getAverageMotion();
    float getMotionVariance();
    
public:
    Detector();
    DetectionResult analyze(float* data, float currentMotion);
    void reset();
};

#endif