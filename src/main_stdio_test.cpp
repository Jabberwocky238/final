#include "mbed.h"

DigitalOut led(LED1);

// 重定向 stdout 到指定的 UART
UnbufferedSerial pc(USBTX, USBRX, 115200);

// 重定向 printf 到硬件串口
namespace mbed {
    FileHandle *mbed_override_console(int fd) {
        return &pc;
    }
}

int main() {
    // 等待一下
    ThisThread::sleep_for(1000ms);

    printf("\r\n\r\n\r\n");
    printf("==========================================\r\n");
    printf("STM32L475 STDIO Redirect Test\r\n");
    printf("==========================================\r\n");
    printf("Using USBTX/USBRX pins\r\n");
    printf("If you see this, it works!\r\n");
    printf("\r\n");

    int counter = 0;

    while(1) {
        led = !led;
        printf("[%d] LED toggle - Test OK\r\n", counter++);
        ThisThread::sleep_for(1000ms);
    }
}
