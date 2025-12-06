#include "mbed.h"

DigitalOut led(LED1);

int main() {
    // 禁用缓冲，确保数据立即发送
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // 等待 USB CDC 稳定 - 很重要！
    ThisThread::sleep_for(2000ms);

    // 发送一些换行符清屏
    printf("\r\n\r\n\r\n\r\n\r\n");
    printf("==========================================\r\n");
    printf("STM32L475 Minimal Test\r\n");
    printf("==========================================\r\n");
    printf("If you see this, serial is working!\r\n");
    printf("\r\n");

    int counter = 0;

    while(1) {
        led = !led;
        printf("[%d] LED toggle - Test message\r\n", counter++);
        fflush(stdout);  // 强制刷新输出
        ThisThread::sleep_for(1000ms);
    }
}
