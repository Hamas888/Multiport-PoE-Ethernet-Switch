/* USER CODE BEGIN Header */
 /*
 ====================================================================================================
 * File:        main.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 05, 2023
 * Brief:       PoE Switch Bootloader main header file.
 * 
 ====================================================================================================
 * License: 
 * This file is licensed under the GNU Affero General Public License (AGPL) v3.0.
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 * https://www.gnu.org/licenses/agpl-3.0.en.html
 * 
 * Commercial licensing: For commercial use of this software, please contact Hamas Saeed at 
 * hamasaeed@gmail.com.
 * 
 * Distributed under the AGPLv3 License. Software is provided "AS IS," without any warranties 
 * or conditions of any kind, either express or implied.
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
#include "save.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
//#define  SERIAL_DEBUG                                                             // Comment This Out TO Disable Serial Output

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

/* For Debugging */
extern uint8_t debugBuff[80];
void SerialDebug(char * data);

/* Function Prototypes */
void updateCheck();
poe_err firmwarePresent();
poe_err copyFirmware(uint32_t source, uint32_t destination);
static void jumpToApplication(void);

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
