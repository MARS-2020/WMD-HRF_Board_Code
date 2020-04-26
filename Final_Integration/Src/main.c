/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "lora.h"
#include "oled.h"
#include "hr.h"
#include "gps.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint8_t data[70] = {0};
uint8_t tempdata[70] = {0};

uint16_t heartrate  = 0;
uint8_t  HR_conf    = 0;
uint16_t SPO2  		= 0;
uint8_t  alg_state  = 0;
uint8_t  alg_status = 0;

char HR_Valid[4];
char SPO2_Valid[4];

char GGA_MESSAGE[50];
char latitude[10];
char longitude[11];
char longDir[2];
char latDir[2];
char fix[2];
char sendMeasurements[27];

uint8_t test[] = "NO GPS FIX";
uint8_t GPS_FLAG = 0;
uint8_t LORA_FLAG = 0;
uint8_t DIM_FLAG = 0;
uint8_t HR_FLAG = 0;
uint8_t SOS_FLAG = 0;

uint8_t contrastLow[]={0x81, 0x0F};
uint8_t contrastHigh[]={0x81, 0xFF};

uint8_t isDim = 0;

uint8_t receive_data[22] = "0";

extern char user[];
extern uint8_t isSelfSetup;
extern uint8_t isOtherSetup;
extern char hr[];
extern uint8_t MAX_BUFF;

uint8_t timer_flag = 0;
uint8_t sos[] = "s,1,\r\n";

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
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  //INITIALIZE MODULES
  turnOnScreen(); //screen
  GPS_INIT(); //gps
  LORA_INIT(); //lora
  HR_APP_MODE(); //call function to put module in application mode
  HR_INIT();
  HR_MFIO_SET();
  shut30101();

  //HAL_UART_Receive_IT(&huart1, tempdata, sizeof(tempdata));
  if(HAL_UART_Receive_DMA(&huart1, tempdata, sizeof(tempdata)) != HAL_OK)
  {
      Error_Handler();
  }
  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //HAL_UART_Receive_IT(&huart1, tempdata, sizeof(tempdata));

	  if ((&huart1)->RxState == HAL_UART_STATE_READY)
	  {
		  HAL_UART_Receive_DMA(&huart1, tempdata, sizeof(tempdata)/sizeof(tempdata[0]));
		  //while(!((&huart1)->RxState == HAL_UART_STATE_READY));
		  //__HAL_UART_FLUSH_DRREGISTER(&huart1);
	  }

	  if(GPS_FLAG)
	  {
		  uint8_t found = 0;
		  uint8_t commaCnt = 0;
		  uint8_t j = 0;

		  if (tempdata[0] != '0') //if there is a fix
		  {
			  for(int i = 0; i < ((sizeof(tempdata) - 1)); i++)
			  {
				  if (commaCnt < 8)
				  {
					  if ((tempdata[i] == '$') && (tempdata[i+1] == 'G'))
					  {
						  found = 1;
					  }
					  if (found)
					  {
						  data[j] = tempdata[i];
						  j++;
						  if (tempdata[i] == ',')
						  {
							  commaCnt++;
						  }
					  }
				  }
				  else
					  break;
			  }

			  if (parseData())
			  {

				  uint8_t test[27]; //CHANGE THIS SIZE ONCE THE PACKET GETS HR DATA
				  for(int i = 0; i < (sizeof(test) / sizeof(test[0])); i++)
				  {
					  test[i] = (uint8_t) sendMeasurements[i];
				  }

				  uint8_t buf[] = "y,1,100,100\r\n";
				  sendPacket(buf, sizeof(buf));
				  //writeReg(RH_RF95_REG_01_OP_MODE, 0x05);
				  //writeReg(RH_RF95_REG_40_DIO_MAPPING1, 0x00);

				  //testScreen();
				  HAL_Delay(500);
				  if (!SOS_FLAG)
				  {
					  start30101();
				  }
			  }
		  }
		  GPS_FLAG = 0;
	  }
	  /*
	  else if(HR_FLAG)
	  {
		  uint8_t received_data[MAX_BUFF];
		  HR_READ(received_data);
		  testScreen();
		  HR_FLAG = 0;
	  }
	  */
	  else if(LORA_FLAG)
	  {
		  receiveData();
		  LORA_FLAG = 0;
	  }
	  else if(DIM_FLAG)
	  {
		  if(isDim)
		  {
			  sendCMD(contrastHigh, (uint16_t)sizeof(contrastHigh));
			  isDim = 0;
		  }
		  else
		  {
			  sendCMD(contrastLow, (uint16_t)sizeof(contrastLow));
			  isDim = 1;
		  }
		  DIM_FLAG = 0;
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
  hi2c1.Init.Timing = 0x00000708;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
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
  htim2.Init.Prescaler = 200000;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1500;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
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
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
   HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
   HAL_NVIC_EnableIRQ(USART1_IRQn);
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
  /* DMA1_Channel2_3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HR_MFIO_GPIO_Port, HR_MFIO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LORA_RST_Pin|HR_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, oled_NSS_Pin|oled_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(oled_RES_GPIO_Port, oled_RES_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : HR_MFIO_Pin */
  GPIO_InitStruct.Pin = HR_MFIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HR_MFIO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LORA_NSS_Pin oled_RES_Pin */
  GPIO_InitStruct.Pin = LORA_NSS_Pin|oled_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LORA_RST_Pin oled_NSS_Pin oled_DC_Pin HR_RESET_Pin */
  GPIO_InitStruct.Pin = LORA_RST_Pin|oled_NSS_Pin|oled_DC_Pin|HR_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LORA_DIO_Pin */
  GPIO_InitStruct.Pin = LORA_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LORA_DIO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIM_Pin SOS_Pin */
  GPIO_InitStruct.Pin = DIM_Pin|SOS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	NVIC_DisableIRQ(EXTI4_15_IRQn);
	NVIC_DisableIRQ(EXTI0_1_IRQn);
	NVIC_DisableIRQ(USART1_IRQn);
	NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);

	GPS_FLAG = 1; //GPS
	__HAL_UART_FLUSH_DRREGISTER(&huart1); // Clear the buffer to prevent overrun

	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	NVIC_DisableIRQ(EXTI4_15_IRQn);
	NVIC_DisableIRQ(EXTI0_1_IRQn);
	NVIC_DisableIRQ(USART1_IRQn);
	NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);

	if(GPIO_Pin == HR_MFIO_Pin) //HR
	{
		uint8_t received_data[MAX_BUFF];
		HR_READ(received_data);

		if(!HR_FLAG)
		{
			NVIC_EnableIRQ(EXTI4_15_IRQn);
		}
		else
		{
			HR_FLAG = 0;
			//testScreen();
			user1Info(heartrate, SPO2);
			NVIC_DisableIRQ(EXTI4_15_IRQn);
			NVIC_EnableIRQ(EXTI0_1_IRQn);
			NVIC_EnableIRQ(USART1_IRQn);
			NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
		}
		return; //since doing heart rate, return
	}
	if (GPIO_Pin == GPIO_PIN_1) //LORA RECEIVE
	{
		LORA_FLAG = 1;
	}
	else if (GPIO_Pin == DIM_Pin) //DIM SCREEN
	{
		DIM_FLAG = 1;
	}
	else if (GPIO_Pin == SOS_Pin)
	{
		HAL_TIM_Base_Start_IT(&htim2);
		SOS_FLAG = 1;
	}

	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim2) {
	NVIC_DisableIRQ(EXTI4_15_IRQn);
	NVIC_DisableIRQ(EXTI0_1_IRQn);
	NVIC_DisableIRQ(USART1_IRQn);
	NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);

	if(SOS_FLAG == 1) {
		HAL_TIM_Base_Stop_IT(htim2);
		clearSOS();
		SOS_FLAG = 0;
	}
	if(timer_flag == 1) {
		__HAL_TIM_CLEAR_IT(htim2 ,TIM_IT_UPDATE);
		timer_flag = 0;
		if(!HAL_GPIO_ReadPin(GPIOB, SOS_Pin)) {
			sendPacket(sos, sizeof(sos));
			sendSOS();
			SOS_FLAG = 1;
		}
	} else {
		timer_flag = 1;
	}

	//NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_EnableIRQ(EXTI4_15_IRQn);
	NVIC_EnableIRQ(EXTI0_1_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
