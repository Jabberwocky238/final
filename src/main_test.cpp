#include "mbed.h"
#include "config.h"
#include "fft_processor.h"
#include "detector.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// LED 指示
DigitalOut led1(LED1);

// 生成测试信号
void generateSineWave(float* buffer, int size, float frequency, float amplitude) {
    for (int i = 0; i < size; i++) {
        buffer[i] = amplitude * sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
}

// 测试 1: FFT 4Hz 震颤
void test_tremor_detection() {
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║  测试 1: 震颤检测 (4Hz)              ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    FFTProcessor fft;
    Detector detector;
    float testData[WINDOW_SIZE];
    
    // 生成 4Hz 信号
    generateSineWave(testData, WINDOW_SIZE, 4.0f, 2.0f);
    
    // FFT 分析
    FrequencyPeak peak = fft.process(testData);
    printf("FFT 分析: 频率 = %.2f Hz, 幅值 = %.3f\n", peak.frequency, peak.magnitude);
    
    // 检测器分析
    DetectionResult result = detector.analyze(testData, 1.0f);
    
    printf("\n结果:\n");
    printf("  震颤检测: %s\n", result.tremorDetected ? "✓ 是" : "✗ 否");
    printf("  震颤强度: %.3f (阈值: %.2f)\n", result.tremorIntensity, TREMOR_THRESHOLD);
    
    if (result.tremorDetected && peak.frequency > 3.0f && peak.frequency < 5.0f) {
        printf("\n✅ 测试通过！\n");
        led1 = 1;
    } else {
        printf("\n❌ 测试失败！\n");
        led1 = 0;
    }
}

// 测试 2: FFT 6Hz 运动障碍
void test_dyskinesia_detection() {
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║  测试 2: 运动障碍检测 (6Hz)          ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    FFTProcessor fft;
    Detector detector;
    float testData[WINDOW_SIZE];
    
    // 生成 6Hz 信号
    generateSineWave(testData, WINDOW_SIZE, 6.0f, 2.0f);
    
    // FFT 分析
    FrequencyPeak peak = fft.process(testData);
    printf("FFT 分析: 频率 = %.2f Hz, 幅值 = %.3f\n", peak.frequency, peak.magnitude);
    
    // 检测器分析
    DetectionResult result = detector.analyze(testData, 1.0f);
    
    printf("\n结果:\n");
    printf("  运动障碍检测: %s\n", result.dyskinesiaDetected ? "✓ 是" : "✗ 否");
    printf("  运动障碍强度: %.3f (阈值: %.2f)\n", result.dyskinesiaIntensity, DYSKINESIA_THRESHOLD);
    
    if (result.dyskinesiaDetected && peak.frequency > 5.0f && peak.frequency < 7.0f) {
        printf("\n✅ 测试通过！\n");
        led1 = 1;
    } else {
        printf("\n❌ 测试失败！\n");
        led1 = 0;
    }
}

// 测试 3: 低频拒绝
void test_low_frequency_rejection() {
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║  测试 3: 低频拒绝 (1Hz)              ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    FFTProcessor fft;
    Detector detector;
    float testData[WINDOW_SIZE];
    
    // 生成 1Hz 信号
    generateSineWave(testData, WINDOW_SIZE, 1.0f, 2.0f);
    
    // 检测器分析
    DetectionResult result = detector.analyze(testData, 1.0f);
    
    printf("\n结果:\n");
    printf("  震颤检测: %s\n", result.tremorDetected ? "✓ 是" : "✗ 否");
    printf("  运动障碍检测: %s\n", result.dyskinesiaDetected ? "✓ 是" : "✗ 否");
    
    if (!result.tremorDetected && !result.dyskinesiaDetected) {
        printf("\n✅ 测试通过！正确拒绝低频信号\n");
        led1 = 1;
    } else {
        printf("\n❌ 测试失败！误报\n");
        led1 = 0;
    }
}

// 测试 4: 高频拒绝
void test_high_frequency_rejection() {
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║  测试 4: 高频拒绝 (10Hz)             ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    FFTProcessor fft;
    Detector detector;
    float testData[WINDOW_SIZE];
    
    // 生成 10Hz 信号
    generateSineWave(testData, WINDOW_SIZE, 10.0f, 2.0f);
    
    // 检测器分析
    DetectionResult result = detector.analyze(testData, 1.0f);
    
    printf("\n结果:\n");
    printf("  运动障碍检测: %s\n", result.dyskinesiaDetected ? "✓ 是" : "✗ 否");
    
    if (!result.dyskinesiaDetected) {
        printf("\n✅ 测试通过！正确拒绝高频信号\n");
        led1 = 1;
    } else {
        printf("\n❌ 测试失败！误报\n");
        led1 = 0;
    }
}

// 测试 5: 静止状态
void test_idle_state() {
    printf("\n╔═══════════════════════════════════════╗\n");
    printf("║  测试 5: 静止状态                    ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    
    Detector detector;
    float testData[WINDOW_SIZE];
    
    // 全零数据
    for (int i = 0; i < WINDOW_SIZE; i++) {
        testData[i] = 0.0f;
    }
    
    // 检测器分析
    DetectionResult result = detector.analyze(testData, 0.0f);
    
    printf("\n结果:\n");
    printf("  震颤检测: %s\n", result.tremorDetected ? "✓ 是" : "✗ 否");
    printf("  运动障碍检测: %s\n", result.dyskinesiaDetected ? "✓ 是" : "✗ 否");
    printf("  运动状态: %d (0=IDLE)\n", result.motionState);
    
    if (!result.tremorDetected && !result.dyskinesiaDetected && result.motionState == MOTION_IDLE) {
        printf("\n✅ 测试通过！\n");
        led1 = 1;
    } else {
        printf("\n❌ 测试失败！\n");
        led1 = 0;
    }
}

// 运行所有测试
void run_all_tests() {
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║   Parkinson's Monitor - 自动测试套件      ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n开始测试...\n");
    
    int passed = 0;
    int total = 5;
    
    // 测试 1
    test_tremor_detection();
    thread_sleep_for(1000);
    
    // 测试 2
    test_dyskinesia_detection();
    thread_sleep_for(1000);
    
    // 测试 3
    test_low_frequency_rejection();
    thread_sleep_for(1000);
    
    // 测试 4
    test_high_frequency_rejection();
    thread_sleep_for(1000);
    
    // 测试 5
    test_idle_state();
    thread_sleep_for(1000);
    
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║            测试完成                        ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n提示: 所有测试应该都通过 ✅\n");
    printf("如果有失败，请检查算法参数\n\n");
}

// 显示菜单
void show_menu() {
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║   Parkinson's Monitor - 测试菜单          ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
    printf("命令:\n");
    printf("  1 - 测试震颤检测 (4Hz)\n");
    printf("  2 - 测试运动障碍检测 (6Hz)\n");
    printf("  3 - 测试低频拒绝 (1Hz)\n");
    printf("  4 - 测试高频拒绝 (10Hz)\n");
    printf("  5 - 测试静止状态\n");
    printf("  a - 运行所有测试\n");
    printf("  h - 显示此菜单\n");
    printf("\n输入命令: ");
}

int main() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Parkinson's Disease Monitor               ║\n");
    printf("║  STM32L475 Discovery Kit IoT               ║\n");
    printf("║  交互式测试程序                            ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
    printf("通过串口连接测试 (115200 baud)\n");
    printf("板载蓝牙: 可通过蓝牙串口访问\n");
    printf("\n初始化中...\n");
    
    led1 = 1;
    thread_sleep_for(500);
    led1 = 0;
    
    printf("✓ 系统就绪\n");
    
    show_menu();
    
    while (1) {
        char cmd = getchar();
        
        if (cmd == '\n' || cmd == '\r') {
            continue;
        }
        
        printf("\n");
        
        switch (cmd) {
            case '1':
                test_tremor_detection();
                show_menu();
                break;
                
            case '2':
                test_dyskinesia_detection();
                show_menu();
                break;
                
            case '3':
                test_low_frequency_rejection();
                show_menu();
                break;
                
            case '4':
                test_high_frequency_rejection();
                show_menu();
                break;
                
            case '5':
                test_idle_state();
                show_menu();
                break;
                
            case 'a':
            case 'A':
                run_all_tests();
                show_menu();
                break;
                
            case 'h':
            case 'H':
                show_menu();
                break;
                
            default:
                printf("未知命令: %c\n", cmd);
                printf("输入 'h' 查看帮助\n");
                printf("输入命令: ");
                break;
        }
    }
}