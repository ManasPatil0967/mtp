09/12/2025

Devices used: 
1. STM32F103C8T6, apparently called Blue Pill board.
2. ESP32 DOIT Devkit V1
3. STLink V2
4. OLED SSD1306 128*32

Connections for UART Bridge (STM32 -> ESP32):
A9 -> RX2
A10 -> TX2
G -> GND

Connections for SSD1306 (STM32 -> SSD1306):
B6 -> SCL
B7 -> SDA
G -> GND
3.3 -> VCC

Steps and Code for SSD1306:
1. Download headers and C files from [https://github.com/afiskon/stm32-ssd1306](afiskon/stm32-ssd1306) and put the headers in Core/Inc and C files in Core/Src.
2. Change the name of ssd_config_template.h to ssd_config.h.
3. Change the height of screen in ssd1306.h.
4. Use the snippet: 
```cpp
#include <string.h>
#include <stdarg.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
// In the user space of code in while(1)
ssd1306_Init();
ssd1306_SetCursor(10, 10);
ssd1306_WriteString("Resampling...", Font_7x10, White);
ssd1306_UpdateScreen();
```
---
10/12/2025

Profiling:
Transmitting data: 1 KB Lorem Ipsum at 8 MHz
1. Blocking on UART: 715089 cycles on average
2. DMA: 6527 cycles

Blocking Code: 
```cpp
HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
```
DMA Code:
```cpp
volatile uint8_t uart_tx_done = 1;
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_tx_done = 1;
    }
}
// In main()
//Claim channel
uart_tx_done = 0;
//Transmit data
HAL_UART_Transmit_DMA(&huart1, (uint8_t*)msg, strlen(msg));
// Wait for DMA to complete (measuring full TX time)
while (!uart_tx_done);
```

Cycle Counter code:
```cpp
uint32_t var;
// Enable Cycle Counter
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
DWT->CYCCNT = 0;
var = DWT->CYCCNT;
```
After removing strlen(msg):
1. Blocking: 708930 cycles
2. DMA: 350

DMA Setup Tips: 
1. Set up DMA in IOC Core
2. ENABLE GLOBAL USART1 NVIC GLOBAL INTERRUPT!
3. Ensure DMA init is before USART init.

The benchmark UART function is as follows:
```cpp
uint32_t Benchmark_UART(void)
{
    static const char msg[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam dolor ligula, sollicitudin tincidunt aliquam ac, mattis faucibus sem. Etiam commodo, sem semper consequat scelerisque, risus felis malesuada neque, vel faucibus nibh risus ut erat. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum nunc magna, tristique vitae tempus quis, egestas eu lectus. Mauris eget ultrices leo. Sed a velit a ante auctor pellentesque eu a mauris. Phasellus eget suscipit metus, vitae sodales libero. Morbi pharetra consequat felis. Sed luctus urna vel arcu hendrerit semper. Vivamus sem tortor, commodo eget quam nec, mollis iaculis diam. Ut placerat non mauris quis rutrum. Ut sit amet ipsum urna. Aenean ultricies ipsum quis mauris pulvinar, vitae vehicula nisl tempus. Praesent luctus est nec sollicitudin mattis. Integer non turpis iaculis, lacinia leo quis, euismod lacus. Curabitur lobortis lorem dolor, vitae volutpat mauris facilisis et. Etiam lacinia in quam eget fringilla. Ut aenean.\r\n";
    uint32_t start, end;
    uint32_t len = strlen(msg);
    // Enable Cycle Counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 1. BLOCKING UART (HAL_UART_Transmit)
    DWT->CYCCNT = 0;
    start = DWT->CYCCNT;
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, HAL_MAX_DELAY);
    end = DWT->CYCCNT;
    uint32_t cycles_blocking = end - start;

    HAL_Delay(5);
    UART_Printf("Blocking UART cycles:       %lu\r\n", cycles_blocking);
    HAL_Delay(5);

    // 2. DMA UART (HAL_UART_Transmit_DMA)
    uart_tx_done = 0;
    DWT->CYCCNT = 0;
    start = DWT->CYCCNT;
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)msg, len);
    end = DWT->CYCCNT;
    uint32_t cycles_dma_start = end - start;

    // Wait for DMA to complete (measuring full TX time)
    while (!uart_tx_done);
    // Measure full end-to-end DMA transmission time
    uint32_t cycles_dma_total = DWT->CYCCNT;
    UART_Printf("DMA UART start cost cycles: %lu\r\n", cycles_dma_start);
    UART_Printf("DMA total cycles:           %lu\r\n", cycles_dma_total);

    return cycles_dma_start;
}
```
