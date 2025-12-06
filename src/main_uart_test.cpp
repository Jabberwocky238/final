#include "mbed.h"
#include <cstring>
#include <cstdio>

DigitalOut led(LED1);

// 使用标准控制台 UART
// 使用 CONSOLE_TX 和 CONSOLE_RX (mbed 定义的默认控制台引脚)
BufferedSerial serial(CONSOLE_TX, CONSOLE_RX, 115200);

void print(const char* str) {
    serial.write(str, strlen(str));
}

int main() {
    // 等待串口稳定
    ThisThread::sleep_for(1000ms);

    print("\r\n\r\n\r\n");
    print("==========================================\r\n");
    print("STM32L475 UART Test\r\n");
    print("==========================================\r\n");
    print("Using Hardware UART (LPUART1)\r\n");
    print("Baud: 115200\r\n");
    print("\r\n");

    int counter = 0;
    char buffer[100];

    while(1) {
        led = !led;

        snprintf(buffer, sizeof(buffer), "[%d] LED toggle - UART working!\r\n", counter++);
        print(buffer);

        ThisThread::sleep_for(1000ms);
    }
}
