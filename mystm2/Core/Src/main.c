/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t text[] = "Lorem ipsum dolor sit amet\r\n";

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void UART_Printf(const char* fmt, ...) {
    char buffer[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 100);
}

void UART_Printf_DMA(const char* fmt, ...) {
    char buffer[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if(HAL_UART_GetState(&huart1) == HAL_UART_STATE_READY) {
		HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buffer, strlen(buffer));
    }
}

volatile uint8_t uart_tx_done = 1;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_tx_done = 1;
    }
}

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

    HAL_Delay(5); // spacing
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



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  ssd1306_Init();
	  ssd1306_SetCursor(0, 0);
	  ssd1306_WriteString("Resampling...", Font_6x8, White);
	  ssd1306_UpdateScreen();
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle LED
//	  UART_Printf("START_DATA\r\n");
//	  for(int i = 0; i < 100; i++) {
//		  UART_Printf("%d\r\n", i); // Simulate DSP data
//		  UART_Printf("1Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam dolor ligula, sollicitudin tincidunt");
//		  UART_Printf("2aliquam ac, mattis faucibus sem. Etiam commodo, sem semper consequat scelerisque, risus felis males");
//		  UART_Printf("3uada neque,");
//		  UART_Printf("4vel faucibus nibh risus ut erat. Orci varius natoque penatibus et magnis dis parturient montes,");
//		  UART_Printf("5nascetur ridiculus mus. Vestibulum nunc magna, tristique vitae tempus quis, egestas eu lectus.");
//		  UART_Printf("6Mauris eget ultrices leo. Sed a velit a ante auctor pellentesque eu a mauris.");
//		  UART_Printf("7Phasellus eget suscipit metus, vitae sodales libero. Morbi pharetra consequat felis. Sed luctus");
//		  UART_Printf("8urna vel arcu hendrerit semper. Vivamus sem tortor, commodo eget quam nec, mollis iaculis diam.");
//		  UART_Printf("9 Ut placerat non mauris quis rutrum. Ut sit amet ipsum urna. Aenean ultricies ipsum quis mauris");
//		  UART_Printf("10pulvinar, vitae vehicula nisl tempus. Praesent luctus est nec sollicitudin mattis. Integer non");
//		  UART_Printf("11turpis iaculis, lacinia leo quis, euismod lacus. Curabitur lobortis lorem dolor, vitae volutpat ");
//		  UART_Printf("12mauris facilisis et. Etiam lacinia in quam eget fringilla. Ut aenean.\n");
//	  }
//	  UART_Printf("\nEND_DATA\r\n");
	  HAL_Delay(100);
	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle LED
//	  uint32_t dma_cyc = Profile_UART_Methods();
//	  UART_Printf("%lu\r\n", dma_cyc);
//	  HAL_Delay(500);        // Wait 500ms
//	  UART_Printf("\nARR:%lu PSC:%lu ARPE:%lu", TIM1->ARR, TIM1->PSC, TIM1->EGR);
//	    uint32_t dmacyc = Profile_UART_Methods();
//
//	    UART_Printf("DMA_CYCLES: %lu\r\n", dmacyc);
//
//	    // Optional: wait for DMA completion before next iteration
//	    while (!uart_dma_ready);
//
//	    HAL_Delay(500);
	    Benchmark_UART();
	    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
