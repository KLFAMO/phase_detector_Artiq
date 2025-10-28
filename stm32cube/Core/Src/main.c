/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "interface.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_PARAM_START_ADDR  ((uint32_t)0x081E0000)  // bank 2, sektor 7
#define FLASH_WORD_SIZE        (32)  // Flash word = 256-bit = 32 bytes
#define BUFFER_SIZE 100
#define MAX(a,b) (((a)>(b))?(a):(b))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */


void Flash_Write_Params(uint32_t address, parameters *data) {
  HAL_FLASH_Unlock();  // Odblokowanie pamięci flash

  FLASH_EraseInitTypeDef eraseInitStruct;
  uint32_t sectorError;

  // Kasowanie sektora przed zapisem
  eraseInitStruct.TypeErase    = FLASH_TYPEERASE_SECTORS;
  eraseInitStruct.Banks        = FLASH_BANK_2;  // **Bank 2**
  eraseInitStruct.Sector       = FLASH_SECTOR_7;  // **Sektor 7**
  eraseInitStruct.NbSectors    = 1;
  eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  if (HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK) {
      HAL_FLASH_Lock();
      return;  // Błąd kasowania
  }
  
  uint64_t *data_ptr = (uint64_t*)data;
  uint64_t flash_word[4];
  for (uint32_t i = 0; i < sizeof(parameters) / 8; i += 4) {
      flash_word[0] = (i < sizeof(parameters) / 8) ? data_ptr[i] : 0xFFFFFFFFFFFFFFFF;
      flash_word[1] = (i + 1 < sizeof(parameters) / 8) ? data_ptr[i + 1] : 0xFFFFFFFFFFFFFFFF;
      flash_word[2] = (i + 2 < sizeof(parameters) / 8) ? data_ptr[i + 2] : 0xFFFFFFFFFFFFFFFF;
      flash_word[3] = (i + 3 < sizeof(parameters) / 8) ? data_ptr[i + 3] : 0xFFFFFFFFFFFFFFFF;

      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address + i * 8, (uint64_t)flash_word) != HAL_OK) {
          HAL_FLASH_Lock();
          return;  // Błąd zapisu
      }
  }

  HAL_FLASH_Lock();  // Zablokowanie pamięci flash
}

void Flash_Read_Params(uint32_t address, parameters *data) {
  memcpy(data, (void*)address, sizeof(parameters));  // Odczytaj całą strukturę
}

uint32_t Flash_Read_Version(uint32_t address) {
  return *(volatile double*)address;  // Odczytaj pierwsze 4 bajty
}


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */
void ExtractMessage(char* rxBuffer, char* txBuffer);
void SetDiv(uint8_t number, uint8_t div);
void HMC984_WriteRegister(uint32_t data, uint8_t address);
uint32_t HMC984_ReadRegister(uint8_t address);

uint8_t rxChar;
  uint8_t rxBuffer[BUFFER_SIZE];
  uint8_t txBuffer[BUFFER_SIZE];
  uint8_t tmpBuffer[BUFFER_SIZE];
  uint16_t index = 0;
  uint8_t helloMsg[] = "\nPhDet>";
  HAL_StatusTypeDef status;
  static uint32_t last_cnt = 0;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern parameters par;
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
  MX_USART3_UART_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  initInterface();

  // read par from flash
  if (par.version != Flash_Read_Version(FLASH_PARAM_START_ADDR)){
    Flash_Write_Params(FLASH_PARAM_START_ADDR, &par);
  }
  else{
    Flash_Read_Params(FLASH_PARAM_START_ADDR, &par);
  }


  HAL_UART_Transmit(&huart4, (uint8_t*)"\r\nPhDet ready\r\n", 15, HAL_MAX_DELAY);

  par.ref.f.val = 0.0f;

  // Start licznika z zewnętrznym zegarem na ETR (PA0)
  HAL_TIM_Base_Start(&htim2);

  // Start okna 100 ms (przerwania)
  HAL_TIM_Base_Start_IT(&htim6);

  // Na start wyzeruj licznik i punkt odniesienia
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  last_cnt = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  status = HAL_UART_Receive(&huart4, &rxChar, 1, HAL_MAX_DELAY);
	  	  	  if (status == HAL_OK)
	  	  	  {
	  	  		  if ((rxChar == '\r' || rxChar == '\n') && index>0){
	  	  			  rxBuffer[index++]='\n';
	  	  			  rxBuffer[index++]='\0';
	  	  			  ExtractMessage((char*) rxBuffer, (char*) txBuffer);
//	  	  			  strcpy(tmpBuffer, helloMsg);
	  	  			  // strcat(tmpBuffer, txBuffer);
	  	  			  // strcpy(txBuffer, tmpBuffer);

	  	  			  HAL_UART_Transmit(&huart4, txBuffer, strlen(txBuffer), HAL_MAX_DELAY);
	  	  			  HAL_UART_Transmit(&huart4, helloMsg, strlen(helloMsg), HAL_MAX_DELAY);
	  	  			  index=0;
	  	  		  }
	  	  		  else{
	  	  			  rxBuffer[index++] = rxChar;
	  	  			  if (index >= BUFFER_SIZE) index = 0;
	  	  		  }
	  	  	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV4;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 1;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 499;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 4999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

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
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart6, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart6, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, DIV2_S1_Pin|DIV2_S2_Pin|DIV2_S0_Pin|DIV1_S2_Pin
                          |DIV1_S1_Pin|DIV1_S0_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PHDET_CEN_GPIO_Port, PHDET_CEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PHDET_NSS_GPIO_Port, PHDET_NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : XVCO_Pin */
  GPIO_InitStruct.Pin = XVCO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(XVCO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIV2_S1_Pin DIV2_S2_Pin DIV2_S0_Pin DIV1_S2_Pin
                           DIV1_S1_Pin DIV1_S0_Pin */
  GPIO_InitStruct.Pin = DIV2_S1_Pin|DIV2_S2_Pin|DIV2_S0_Pin|DIV1_S2_Pin
                          |DIV1_S1_Pin|DIV1_S0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PHDET_CEN_Pin */
  GPIO_InitStruct.Pin = PHDET_CEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PHDET_CEN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PHDET_NSS_Pin */
  GPIO_InitStruct.Pin = PHDET_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PHDET_NSS_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
      if (par.pd.read.val > 0) {
          par.pd.rval.val = HMC984_ReadRegister((uint8_t)par.pd.reg.val);
          par.pd.read.val = 0;
      }
    }
}

void HMC984_WriteRegister(uint32_t data, uint8_t address) {
    uint8_t buffer[5];  // 40-bits
    uint8_t chip_address = 0b100;

    buffer[0] = (data >> 22) & 0xFF; // MSB
    buffer[1] = (data >> 14) & 0xFF;
    buffer[2] = (data >> 6) & 0xFF;
    buffer[3] = (data & 0x3F) << 2; //LSB
    buffer[3] |= (address >> 5) & 0x03;
    buffer[4] = (address << 3) | chip_address;

//    HAL_GPIO_WritePin(PHDET_CEN_GPIO_Port, PHDET_CEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PHDET_NSS_GPIO_Port, PHDET_NSS_Pin, GPIO_PIN_RESET);

    // Wyślij dane przez SPI2 (40-bitów w 5 bajtach)
    HAL_SPI_Transmit(&hspi2, buffer, 5, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(PHDET_NSS_GPIO_Port, PHDET_NSS_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(PHDET_CEN_GPIO_Port, PHDET_CEN_Pin, GPIO_PIN_RESET);
}

uint32_t HMC984_ReadRegister(uint8_t address) {
    uint8_t tx_buffer[5] = {0};
    uint8_t rx_buffer[5];
    uint32_t received_data = 0;

    // Phase 1: set reg addr to Reg 00h
    HMC984_WriteRegister(0, address);

    // Phase 2: read data
//    HAL_GPIO_WritePin(PHDET_CEN_GPIO_Port, PHDET_CEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PHDET_NSS_GPIO_Port, PHDET_NSS_Pin, GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(&hspi2, tx_buffer, rx_buffer, 5, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(PHDET_NSS_GPIO_Port, PHDET_NSS_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(PHDET_CEN_GPIO_Port, PHDET_CEN_Pin, GPIO_PIN_RESET);

    received_data = ((uint32_t)rx_buffer[0] << 22) | ((uint32_t)rx_buffer[1] << 14) |
                        ((uint32_t)rx_buffer[2] << 6) | ((uint32_t)rx_buffer[3] >> 2);

    return received_data;
}

void SetDiv(uint8_t number, uint8_t div)
{
  GPIO_PinState s0, s1, s2;

  if (div < 2) {
      div = 1;
      s0 = GPIO_PIN_SET;   s1 = GPIO_PIN_SET;   s2 = GPIO_PIN_SET;
  } else if (div < 4) {
      div = 2;
      s0 = GPIO_PIN_RESET; s1 = GPIO_PIN_SET;   s2 = GPIO_PIN_SET;
  } else if (div < 8) {
      div = 4;
      s0 = GPIO_PIN_SET;   s1 = GPIO_PIN_SET;   s2 = GPIO_PIN_RESET;
  } else if (div < 16) {
      div = 8;
      s0 = GPIO_PIN_SET;   s1 = GPIO_PIN_RESET; s2 = GPIO_PIN_RESET;
  } else {
      div = 16;
      s0 = GPIO_PIN_RESET; s1 = GPIO_PIN_RESET; s2 = GPIO_PIN_RESET;
  }

  if (number == 2) {
      HAL_GPIO_WritePin(DIV1_S0_GPIO_Port, DIV1_S0_Pin, s0);
      HAL_GPIO_WritePin(DIV1_S1_GPIO_Port, DIV1_S1_Pin, s1);
      HAL_GPIO_WritePin(DIV1_S2_GPIO_Port, DIV1_S2_Pin, s2);
  }
  if (number == 1) {
        HAL_GPIO_WritePin(DIV2_S0_GPIO_Port, DIV2_S0_Pin, s0);
        HAL_GPIO_WritePin(DIV2_S1_GPIO_Port, DIV2_S1_Pin, s1);
        HAL_GPIO_WritePin(DIV2_S2_GPIO_Port, DIV2_S2_Pin, s2);
  }  
}

void ExtractMessage(char* rxBuffer, char* txBuffer)
{
    cmd_string_interpret(rxBuffer, txBuffer);
    txBuffer[BUFFER_SIZE - 1] = '\0';
    //update_array();
}

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

#ifdef  USE_FULL_ASSERT
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
