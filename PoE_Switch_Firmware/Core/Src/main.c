/* USER CODE BEGIN Header */
/**
 ====================================================================================================
 * File:		main.c
 * Author:		AMS-IOT
 * Version:	    Rev_1.0.0
 * Date:		Dec 05, 2023
 * Brief:		This file contains the application
 *
 ====================================================================================================
 * Attention:
 *                         COPYRIGHT 2021 AMS-IOT Pvt Ltd.
 *
 * Licensed under ************* License Agreement V2, (the "License");
 * Third Party may not use this file except in compliance with the License.
 * Third Party may obtain a copy of the License at:
 *
 *
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 =====================================================================================================
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ksz9897.h"
#include "tps23881.h"
#include "server.h"
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

SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

extern struct netif gnetif;				 // Generic data structure for lwIP network interfaces
uint8_t* portsConnectionStatus;          // Ethernet port connection status
DeviceSettings poeSettings;              // Structure To Stores the device settings
TPS23881 PSE;                            // Structure for TPS23881
uint8_t malfPort = 0;                    // Stores the malfunctioning port
bool systemResetFlag = false;            // System  reset flag
bool systemBootFlag = false;             // System boot flag
bool pseIntFlag = false;                 // PSE Interrupt Flag
poe_err error;                           // For error handling

#ifdef SERIAL_DEBUG
uint8_t debugBuff[80];                   // Buffer for serial debugging
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
#ifdef SERIAL_DEBUG
static void MX_USART2_UART_Init(void);
#endif
static void MX_SPI3_Init(void);
static void MX_TIM10_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

poe_err readSettings();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  Callback function for GPIO interrupts
  * @param  GPIO pin
  */

void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin)
{
#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "\n---------Interrupt Detected\r\n") );
#endif
	poe_err ret;

	// Checks Interrupt From TPS23381
	if(GPIO_Pin == TPS_INT_Pin)
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------SourceTPS23381\r\n") );
#endif
		// Checks If TPS Initialized
		if(PSE.initStatus)
		{
			pseIntFlag = true;
			PSE.interruptPin = 1;

			// Extracting Port That Generated Interrupt
			uint8_t port = TPS23881_InterruptHandler(&PSE, DEVICE_ADDR_A);

			// If POE Device Connected and Classified
			if(tpsPortsPowerStatus[port - 1] && port != 0)
			{
				// Assigning Power Class to Port
				ret = TPS23881_AssignPortClass(&PSE, port, tpsPortsClassificationBuffer[port - 1], DEVICE_ADDR_A);

				if(ret == POE_OK)
				{
					// Enabling the Port Power
					ret = TPS23881_PowerEnable(&PSE, port, Enable, DEVICE_ADDR_A);
				}

				if(ret == POE_OK)
				{
					// Checking For Ethernet Switch KSZ9897 Interrupts
					portsConnectionStatus = ksz9897InterruptHandler(HSPI);

					if(portsConnectionStatus[port - 1])
					{
						// Implement your logic if Ethernet device connected

						ret = ksz9897SetPortState(HSPI, port, SWITCH_PORT_STATE_FORWARDING);
					}
					else if(!portsConnectionStatus[port - 1])
					{
						// Implement your logic if Ethernet device not connected

						ret = ksz9897SetPortState(HSPI, port, SWITCH_PORT_STATE_DISABLED);

					}

					// Handle Error
					if(ret != POE_OK)
					{

					}
				}

			}

			// If POE Device Disconnected
			else if(!tpsPortsPowerStatus[port - 1] && port != 0)
			{
				//Disabling the Port Power
				ret = TPS23881_PowerEnable(&PSE, port, Disable, DEVICE_ADDR_A);

				// Handle Error
				if(ret != POE_OK)
				{

				}
			}
		}
	}
	else if(GPIO_Pin == KSZ_INT_Pin)
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Source KSZ9897\r\n") );
#endif
		// Checks If PSE Interrupted Or Not Which Indicate Only Ethernet Device
		if(!pseIntFlag)
		{
			// Checking For Ethernet Switch KSZ9897 Interrupts
			portsConnectionStatus = ksz9897InterruptHandler(HSPI);
			for(uint8_t port = 1; port <= 5; port++)
			{
				if(portsConnectionStatus[port - 1]) {
					ret = ksz9897SetPortState(HSPI, port, SWITCH_PORT_STATE_FORWARDING);
				}
				else {
					ret = ksz9897SetPortState(HSPI, port, SWITCH_PORT_STATE_DISABLED);
				}

				// Handle Error
				if(ret != POE_OK)
				{

				}
			}
		}
	}
}

/**
  * @brief  Callback function for Timer interrupts
  * @param  GPIO pin
  */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if(htim == &htim10)
	{
		//Run check for any PD malfunctioning
		malfPort = TPS23881_PortAutoRecovery(&PSE, DEVICE_ADDR_A);
	}
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
  MX_LWIP_Init();

  // Turning Amber Light On Indicating LWIP Initialized Successfully
  HAL_GPIO_WritePin(Amber_GPIO_Port, Amber_Pin, GPIO_PIN_SET);

#ifdef SERIAL_DEBUG
  MX_USART2_UART_Init();
#endif

  MX_SPI3_Init();
  MX_I2C1_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
#ifdef SERIAL_DEBUG
  SerialDebug( strcpy((char*) debugBuff, "---------Starting\r\n") );
#endif

  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(TPS_RST_GPIO_Port, TPS_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(TPS_RST_GPIO_Port, TPS_RST_Pin, GPIO_PIN_SET);
#ifdef SERIAL_DEBUG
  SerialDebug( strcpy((char*) debugBuff, "\n---------Reseting PoE Switch\r\n") );
#endif
  // Loading Settings From MCU Flash
  if(readSettings() != POE_OK)
  {
	  // Handle Error
  }

  // Initializing HTTP Server
  http_server_init();

#ifdef SERIAL_DEBUG
  SerialDebug( strcpy((char*) debugBuff, "---------Starting Server\r\n") );
#endif

  // Turning Orange Light On Indicating Settings Loaded and Server Initialized Successfully
  HAL_GPIO_WritePin(Orange_GPIO_Port, Orange_Pin, GPIO_PIN_SET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	/*	Initializing KSZ98997	*/

	if(ksz9897Init(HSPI, false) == POE_OK)
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------KSZ9897 Initialization Successful\r\n") );
#endif
		// Turning Green Light On Indicating KSZ9897 Initialized Successfully
		HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
	}
	else
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------KSZ9897 Initialization Unsuccessful\r\n") );
#endif
		// Turning Red Light On Indicating KSZ9897 Initialization Failed
		HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
	}

	/*	Initializing KSZ98997	*/

	if(TPS23881_init(&PSE, HI2C, OPERATING_MODE_SEMI_AUTO) == POE_OK)
	{
	  // Clear the interrupt and event statuses while reading
	  error = GetAllInterruptStatus(&PSE, DEVICE_ADDR_A);
	  error = GetAllInterruptStatus(&PSE, DEVICE_ADDR_B);

	  error = TPS23881_GetDeviceInterruptStatus(&PSE, DEVICE_ADDR_A);
	  error = TPS23881_GetDeviceInterruptStatus(&PSE, DEVICE_ADDR_B);

	  // Handle Error
	  if(error != POE_OK)
	  {

	  }

#ifdef SERIAL_DEBUG
	  SerialDebug( strcpy((char*) debugBuff, "---------TPS23881 Initialization Successful\r\n\n\n") );
#endif

	  HAL_TIM_Base_Start_IT(&htim10);
	}
	else
	{
#ifdef SERIAL_DEBUG
	  SerialDebug( strcpy((char*) debugBuff, "---------TPS23881 Initialization Unsuccessful\r\n\n") );
#endif
	}
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  // Checks If PSE Interrupt Flag Set Then Reset It
	  if(pseIntFlag) {
		  pseIntFlag = false;
	  }

	  // To Ping The PoE Switch
	  ethernetif_input(&gnetif);
	  sys_check_timeouts();

	  // Check If System Reset Flag True Then Rest The Device
	  if(systemResetFlag)
	  {
		  NVIC_SystemReset();
	  }

	  // Check If System Boot Flag False Then Confirms Successful Boot
	  if(!systemBootFlag)
	  {
		  systemBootFlag = true;
		  memset(poeSettings.boot, '\0', 10);
		  strcpy(poeSettings.boot , "normal");
		  error = writeSettingsToFlash(&poeSettings);

		  if(error != POE_OK) {
			  // Handle error
		  }

		  // In Case Of Update Rests The Device
		  if(strcmp(poeSettings.update, "true") == 0)
		  {
			  NVIC_SystemReset();
		  }
	  }

	  // Check If Malfunctioning Port Not Zero Then Take Action Against It
	  if(malfPort)
	  {
		  error = TPS23881_PowerEnable(&PSE, malfPort, Disable, DEVICE_ADDR_A);
		  error = TPS23881_RestartDetectClass(&PSE, malfPort, DEVICE_ADDR_A);
		  malfPort = 0;

		  // Handle Error
		  if(error != POE_OK)
		  {

		  }
	  }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 60000-1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
#ifdef SERIAL_DEBUG
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
#endif
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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI_CS_Pin|TPS_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Red_Pin|Amber_Pin|Green_Pin|Orange_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, TPS_OSS_Pin|Reset_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SPI_CS_Pin TPS_RST_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin|TPS_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : TPS2373_TPH_Pin TPS2373_TPL_Pin TPS2373_BT_Pin */
  GPIO_InitStruct.Pin = TPS2373_TPH_Pin|TPS2373_TPL_Pin|TPS2373_BT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : Red_Pin Amber_Pin Green_Pin Orange_Pin */
  GPIO_InitStruct.Pin = Red_Pin|Amber_Pin|Green_Pin|Orange_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : TPS_OSS_Pin Reset_Pin */
  GPIO_InitStruct.Pin = TPS_OSS_Pin|Reset_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : KSZ_INT_Pin */
  GPIO_InitStruct.Pin = KSZ_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KSZ_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LVTTL_CLK_Pin LVTTL_SYNC_Pin */
  GPIO_InitStruct.Pin = LVTTL_CLK_Pin|LVTTL_SYNC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : TPS_INT_Pin */
  GPIO_InitStruct.Pin = TPS_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TPS_INT_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Reads the saved settings
 **/

poe_err readSettings()
{
	poe_err ret = POE_OK;
	DeviceSettings tempSettings;

	// Reading saved settings
	readSettingsFromFlash(&tempSettings);

	// Checking settings status
	if(strcmp(tempSettings.def, "false") == 0)
	{
		// Quick check for updated firmware
		if(strcmp(tempSettings.fV, FIRMWARE_VERSION) != 0)
		{
			memset(tempSettings.fV, '\0', 10);
			strcpy(tempSettings.fV, FIRMWARE_VERSION);
			if(tempSettings.size != FIRMWARE_SIZE)
			{
				tempSettings.size = FIRMWARE_SIZE;
			}

			if(writeSettingsToFlash(&tempSettings) != POE_OK) {
				ret = POE_ERR;
			}

		}

		// If default is false load the settings
		poeSettings = tempSettings;

#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Settings Loaded Successfully\r\n") );
#endif
		// Return Error
		return ret;
	}
	else
	{
		// Otherwise load the default settings
		for(int i = 0; i < 5; i++)
		{
			strcpy(poeSettings.speed[i] ,"100 MBps");
			strcpy(poeSettings.status[i] ,"Forwarding");
			strcpy(poeSettings.pdconfig[i] , "No");
			strcpy(poeSettings.pdenable[i] , "disable");
		}
		memset(&poeSettings.portACL, '\0', sizeof(poeSettings.portACL));
		strcpy(poeSettings.username, "admin");
		strcpy(poeSettings.password , "admin");
		strcpy(poeSettings.def , "false");
		strcpy(poeSettings.boot , "normal");
		strcpy(poeSettings.fV , FIRMWARE_VERSION);
		strcpy(poeSettings.update , "false");
		poeSettings.size = FIRMWARE_SIZE;
		// Save the default settings in the memory
		ret = writeSettingsToFlash(&poeSettings);

#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Settings Not Present Loading Default\r\n") );
#endif
		return ret;
	}
}

/**
  * @brief  Print serial logs
  * @param  Pointer to data
  */
#ifdef SERIAL_DEBUG
void SerialDebug(char *data)
{

    HAL_UART_Transmit(&huart2, (uint8_t*)data, strlen(data), HAL_MAX_DELAY);

}
#endif
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
