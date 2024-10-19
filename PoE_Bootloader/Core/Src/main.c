/* USER CODE BEGIN Header */

/**
 =====================================================================================================
 * File:		main.c
 * Author:		AMS-IOT
 * Version:	    Rev_1.0.0
 * Date:		Dec 05, 2023
 * Brief:		This file contains the boot loader
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include "save.h"

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
#ifdef SERIAL_DEBUG
UART_HandleTypeDef huart2;
#endif
/* USER CODE BEGIN PV */

DeviceSettings poeSettings;

#ifdef SERIAL_DEBUG
uint8_t debugBuff[80];
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
#ifdef SERIAL_DEBUG
static void MX_USART2_UART_Init(void);
#endif
/* USER CODE BEGIN PFP */

void updateCheck();

poe_err firmwarePresent();

poe_err copyFirmware(uint32_t source, uint32_t destination);

static void jumpToApplication(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
#ifdef SERIAL_DEBUG
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  SerialDebug( strcpy((char*) debugBuff, "\r\n") );
  SerialDebug( strcpy((char*) debugBuff, "---------Bootloader Running\r\n") );
#endif
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  readSettingsFromFlash(&poeSettings);

  updateCheck();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Print serial logs
  * @param[in]  data Pointer to data buffer that needs to be print
  */

#ifdef SERIAL_DEBUG
void SerialDebug(char * data)
{
	HAL_UART_Transmit(&huart2, debugBuff, strlen((char*)debugBuff), HAL_MAX_DELAY);
}
#endif

/**
  * @brief  Handles the update process
  */

void updateCheck()
{
	if(firmwarePresent() == POE_OK)
	{
		poe_err ret;
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Checking For Update Request\r\n") );
#endif

		if((strcmp(poeSettings.update,"true") == 0) && (strcmp(poeSettings.boot,"required") == 0))
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------New Update Is Present\r\n") );
#endif
			ret = eraseFlashSector(FLASH_SECTOR_5);
			ret = eraseFlashSector(FLASH_SECTOR_6);

			if(ret == POE_OK) {
				ret = copyFirmware(NEW_FIRMWARE_START_ADDR, APPLICATION_START_ADDR);
			}

			if(ret == POE_OK) {
				ret = eraseFlashSector(FLASH_SECTOR_9);
				ret = eraseFlashSector(FLASH_SECTOR_10);
			}

			if(ret != POE_OK) {
				// Handle Error
			}

			else {
#ifdef SERIAL_DEBUG
				SerialDebug( strcpy((char*) debugBuff, "---------Firmware Is Updated\r\n") );
				SerialDebug( strcpy((char*) debugBuff, "---------Confirming Update\r\n") );
#endif
				jumpToApplication();
			}
		}
		else if((strcmp(poeSettings.update,"true") == 0) && (strcmp(poeSettings.boot,"normal") == 0))
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------Firmware Updated Is Successful\r\n") );
			SerialDebug( strcpy((char*) debugBuff, "---------Creating New Backup\r\n") );
#endif
			ret = eraseFlashSector(FLASH_SECTOR_7);
			ret = eraseFlashSector(FLASH_SECTOR_8);

			if(ret == POE_OK) {
				ret = copyFirmware(APPLICATION_START_ADDR, APPLICATION_BACKUP_START_ADDR);
			}

			if(ret == POE_OK) {
				strcpy(poeSettings.update , "false");
				ret = writeSettingsToFlash(&poeSettings);
			}

			if(ret != POE_OK) {
				// Handle Error
			}

			else {
#ifdef SERIAL_DEBUG
				SerialDebug( strcpy((char*) debugBuff, "---------Backup Created Booting Device\r\n") );
				SerialDebug( strcpy((char*) debugBuff, "---------Device Firmware version: ") );
				SerialDebug( strcpy((char*) debugBuff, poeSettings.fV) );
				SerialDebug( strcpy((char*) debugBuff, "\r\n") );
#endif
				jumpToApplication();
			}
		}
		else if(strcmp(poeSettings.def,"false") != 0)
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------First Boot Detected\r\n") );
			SerialDebug( strcpy((char*) debugBuff, "---------Creating Backup\r\n") );
#endif
			ret = eraseFlashSector(FLASH_SECTOR_7);
			ret = eraseFlashSector(FLASH_SECTOR_8);

			if(ret == POE_OK) {
				ret = copyFirmware(APPLICATION_START_ADDR, APPLICATION_BACKUP_START_ADDR);
			}

			if(ret != POE_OK) {
				// Handle Error
			}

			else {
#ifdef SERIAL_DEBUG
				SerialDebug( strcpy((char*) debugBuff, "---------Backup Created\r\n") );
				SerialDebug( strcpy((char*) debugBuff, "\r\n") );
#endif
				jumpToApplication();
			}
		}
		else
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------No Update Is Present\r\n") );
			SerialDebug( strcpy((char*) debugBuff, "---------Device Firmware version: ") );
			SerialDebug( strcpy((char*) debugBuff, poeSettings.fV) );
			SerialDebug( strcpy((char*) debugBuff, "\r\n") );
#endif
			jumpToApplication();
		}
	}

	else
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Fatal Error Firmware not Present\r\n") );
#endif
	}
}

/**
  * @brief  Print serial logs
  * @return error code
  */

poe_err firmwarePresent()
{
    uint8_t checkData[50];
    uint8_t count = 1;

    readFlash(APPLICATION_START_ADDR, checkData, sizeof(checkData));

    for (size_t i = 0; i < 50; i++)
    {
        if (checkData[i] == 0xFF)
        {
            count++;
        }
    }

    // Return Error
    if(count >= 50)
    {
    	return POE_ERR;
    }

    return POE_OK;
}

/**
  * @brief  Copy firmware from one location to other
  * @param[in]  Source address
  * @param[in]  Destination address
  * @return  error code
  */

poe_err copyFirmware(uint32_t source, uint32_t destination)
{
	int chunks;
	if(poeSettings.size > 10)
	{
		chunks = poeSettings.size;
	}
	else
	{
		chunks = 256;
	}
	uint8_t buffer[1024];
	uint32_t updateAdress = 0;
	uint32_t applicationAdress = 0;
	poe_err ret = POE_OK;

	for(int i = 0; i < chunks; i++)
	{
		updateAdress = source + (i * 0x400);
		applicationAdress = destination + (i* 0x400);
		readFlash(updateAdress, buffer, sizeof(buffer));
		ret = writeFlash(applicationAdress, buffer, sizeof(buffer));

	}

	return ret;
}

/**
  * @brief  Start the main application
  */

static void jumpToApplication(void)
{
#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "---------Starting Device\r\n") );
	SerialDebug( strcpy((char*) debugBuff, "\r\n\n") );
#endif
	void (*app_reset_handler)(void) = (void*)(*((volatile uint32_t*) (0x08020000 + 4U)));

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	HAL_RCC_DeInit();
	HAL_DeInit();
	__set_MSP(*(volatile uint32_t*) 0x08020000);
    SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	app_reset_handler();
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
