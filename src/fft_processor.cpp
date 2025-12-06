#include "fft_processor.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

FFTProcessor::FFTProcessor() {
    // 初始化数组
    for (int i = 0; i < WINDOW_SIZE; i++) {
        realData[i] = 0;
        imagData[i] = 0;
    }
    for (int i = 0; i < WINDOW_SIZE / 2; i++) {
        magnitudes[i] = 0;
    }
}

void FFTProcessor::bitReverse(float* real, float* imag, int n) {
    int j = 0;
    for (int i = 0; i < n - 1; i++) {
        if (i < j) {
            // 交换 real[i] 和 real[j]
            float temp = real[i];
            real[i] = real[j];
            real[j] = temp;
            
            // 交换 imag[i] 和 imag[j]
            temp = imag[i];
            imag[i] = imag[j];
            imag[j] = temp;
        }
        
        int k = n / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
}

void FFTProcessor::fft(float* real, float* imag, int n) {
    // Cooley-Tukey FFT 算法
    bitReverse(real, imag, n);
    
    // FFT 计算
    for (int len = 2; len <= n; len *= 2) {
        float angle = -2.0f * M_PI / len;
        float wlenReal = cosf(angle);
        float wlenImag = sinf(angle);
        
        for (int i = 0; i < n; i += len) {
            float wReal = 1.0f;
            float wImag = 0.0f;
            
            for (int j = 0; j < len / 2; j++) {
                float uReal = real[i + j];
                float uImag = imag[i + j];
                
                float vReal = real[i + j + len / 2];
                float vImag = imag[i + j + len / 2];
                
                float tReal = wReal * vReal - wImag * vImag;
                float tImag = wReal * vImag + wImag * vReal;
                
                real[i + j] = uReal + tReal;
                imag[i + j] = uImag + tImag;
                
                real[i + j + len / 2] = uReal - tReal;
                imag[i + j + len / 2] = uImag - tImag;
                
                float wTempReal = wReal;
                wReal = wReal * wlenReal - wImag * wlenImag;
                wImag = wTempReal * wlenImag + wImag * wlenReal;
            }
        }
    }
}

FrequencyPeak FFTProcessor::process(float* data) {
    // 复制数据并应用汉宁窗
    for (int i = 0; i < WINDOW_SIZE; i++) {
        float window = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (WINDOW_SIZE - 1)));
        realData[i] = data[i] * window;
        imagData[i] = 0;
    }
    
    // 执行 FFT
    fft(realData, imagData, WINDOW_SIZE);
    
    // 计算幅值
    for (int i = 0; i < WINDOW_SIZE / 2; i++) {
        magnitudes[i] = sqrtf(realData[i] * realData[i] + imagData[i] * imagData[i]) / (WINDOW_SIZE / 2.0f);
    }
    
    // 找出最大峰值 (1-10Hz 范围)
    int minBin = (int)(1.0f * WINDOW_SIZE / SAMPLE_RATE);
    int maxBin = (int)(10.0f * WINDOW_SIZE / SAMPLE_RATE);
    
    FrequencyPeak peak;
    peak.magnitude = 0;
    peak.frequency = 0;
    
    for (int i = minBin; i < maxBin && i < WINDOW_SIZE / 2; i++) {
        if (magnitudes[i] > peak.magnitude) {
            peak.magnitude = magnitudes[i];
            peak.frequency = (float)i * SAMPLE_RATE / WINDOW_SIZE;
        }
    }
    
    return peak;
}

FrequencyPeak FFTProcessor::findPeakInRange(float minFreq, float maxFreq) {
    int minBin = (int)(minFreq * WINDOW_SIZE / SAMPLE_RATE);
    int maxBin = (int)(maxFreq * WINDOW_SIZE / SAMPLE_RATE);
    
    FrequencyPeak peak;
    peak.magnitude = 0;
    peak.frequency = 0;
    
    for (int i = minBin; i <= maxBin && i < WINDOW_SIZE / 2; i++) {
        if (magnitudes[i] > peak.magnitude) {
            peak.magnitude = magnitudes[i];
            peak.frequency = (float)i * SAMPLE_RATE / WINDOW_SIZE;
        }
    }
    
    return peak;
}