#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "si5351.h"

void Error_Handler(void) {
    __disable_irq();
    while(1);
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
    HAL_RCC_EnableCSS();
}

void process_command(char *cmd) {
    if(strncmp(cmd, "set_freq(", 9) == 0) {
        char *start = strchr(cmd, '(');
        char *end = strchr(start, ')');
        if(start && end && end > start) {
            char freq_str[10];
            strncpy(freq_str, start + 1, end - start - 1);
            freq_str[end - start - 1] = '\0';
            uint32_t freq = atoi(freq_str);
            si5351_SetupCLK0(freq, SI5351_DRIVE_STRENGTH_4MA);
            si5351_EnableOutputs(1 << 0);
            char buffer[20];
            int n_buffer = sprintf(buffer, "Set to %lu Hz!\r\n", freq);
            HAL_UART_Transmit(&huart1, buffer, n_buffer, 100);
        }
    }
}

int main(void) {
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();

    HAL_UART_Transmit(&huart1, "Serial VFO!\r\n", 13, 100);
    si5351_Init(978);
    HAL_UART_Transmit(&huart1, "Init done!\r\n", 12, 100);
    char rx_buffer[30];

    while(1) {
        uint8_t ch;
        static uint16_t idx;
        if(HAL_UART_Receive(&huart1, &ch, 1, 10) == HAL_OK) {
            if(ch == '\n' || ch == '\r') {
                rx_buffer[idx] = '\0';
                process_command(rx_buffer);
                idx = 0;
                memset(rx_buffer, 0, sizeof(rx_buffer));
            }
            else if(idx < sizeof(rx_buffer) - 1) {
                rx_buffer[idx++] = ch;
            }
        }
    }
}
