/* USER CODE BEGIN Header */
/**
 ====================================================================================================
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//#define  SERIAL_DEBUG                   // Comment This Out TO Disable Serial Output

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

// For debugging
extern uint8_t debugBuff[80];
void SerialDebug(char * data);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_15
#define LED_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

#define NEW_FIRMWARE_START_ADDR       0x080A0000
#define APPLICATION_START_ADDR        0x08020000
#define APPLICATION_BACKUP_START_ADDR 0x08060000
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
