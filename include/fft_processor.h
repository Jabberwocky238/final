#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include "mbed.h"
#include "config.h"
#include <cmath>

struct FrequencyPeak {
    float frequency;
    float magnitude;
};

class FFTProcessor {
private:
    float realData[WINDOW_SIZE];
    float imagData[WINDOW_SIZE];
    float magnitudes[WINDOW_SIZE / 2];
    
    void fft(float* real, float* imag, int n);
    void bitReverse(float* real, float* imag, int n);
    
public:
    FFTProcessor();
    FrequencyPeak process(float* data);
    FrequencyPeak findPeakInRange(float minFreq, float maxFreq);
};

#endif