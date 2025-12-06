#include "mbed.h"

// LED
DigitalOut led(LED1);

// 重定向 stdout 到硬件串口 (修复串口输出问题)
UnbufferedSerial pc(USBTX, USBRX, 115200);

namespace mbed {
    FileHandle *mbed_override_console(int fd) {
        return &pc;
    }
}

int main() {
    // 等待串口稳定
    ThisThread::sleep_for(1000ms);
    
    // 发送初始化消息
    printf("\r\n\r\n\r\n");
    printf("=================================\r\n");
    printf("STM32L475 串口测试\r\n");
    printf("=================================\r\n");
    printf("如果你看到这条消息，说明串口工作正常！\r\n");
    printf("\r\n");
    
    // LED 快速闪烁 5 次表示启动
    printf("LED 启动闪烁...\r\n");
    for (int i = 0; i < 5; i++) {
        led = 1;
        ThisThread::sleep_for(200ms);
        led = 0;
        ThisThread::sleep_for(200ms);
    }
    printf("启动完成！\r\n\r\n");
    
    printf("测试菜单:\r\n");
    printf("  按 '1' - LED 测试\r\n");
    printf("  按 'a' - 自动测试\r\n");
    printf("  按 'h' - 显示帮助\r\n");
    printf("\r\n请输入命令: ");
    fflush(stdout);
    
    while(1) {
        // 等待用户输入
        int c = getchar();
        
        if (c == EOF || c < 0) {
            ThisThread::sleep_for(100ms);
            continue;
        }
        
        char cmd = (char)c;
        
        // 忽略回车换行
        if (cmd == '\r' || cmd == '\n') {
            continue;
        }
        
        // 回显
        printf("%c\r\n\r\n", cmd);
        
        switch (cmd) {
            case '1':
                printf("=== LED 测试 ===\r\n");
                led = 1;
                printf("LED 已点亮\r\n");
                ThisThread::sleep_for(1000ms);
                led = 0;
                printf("LED 已熄灭\r\n");
                printf("✅ 测试完成\r\n\r\n");
                break;
                
            case 'a':
            case 'A':
                printf("=== 自动测试 ===\r\n");
                for (int i = 0; i < 3; i++) {
                    printf("测试 %d/3...\r\n", i + 1);
                    led = 1;
                    ThisThread::sleep_for(500ms);
                    led = 0;
                    ThisThread::sleep_for(500ms);
                }
                printf("✅ 自动测试完成\r\n\r\n");
                break;
                
            case 'h':
            case 'H':
                printf("=== 帮助 ===\r\n");
                printf("可用命令:\r\n");
                printf("  1 - LED 单次测试\r\n");
                printf("  a - 自动测试（3次闪烁）\r\n");
                printf("  h - 显示此帮助\r\n");
                printf("\r\n");
                break;
                
            default:
                printf("未知命令: '%c' (ASCII: %d)\r\n", cmd, cmd);
                printf("按 'h' 查看帮助\r\n\r\n");
                break;
        }
        
        printf("请输入命令: ");
        fflush(stdout);
    }
}