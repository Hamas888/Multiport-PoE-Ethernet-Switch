/* USER CODE BEGIN Header */
/*
 ====================================================================================================
 * File:        main.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 05, 2023
 * Brief:       main application header file
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
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "save.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

#define  SERIAL_DEBUG                   // Comment This Out TO Disable Serial Output

extern SPI_HandleTypeDef hspi3;

#ifdef SERIAL_DEBUG
extern UART_HandleTypeDef huart2;
#endif

extern I2C_HandleTypeDef hi2c1;

extern DeviceSettings poeSettings;

extern uint8_t* portsConnectionStatus;

extern bool systemResetFlag;
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

#ifdef SERIAL_DEBUG
extern uint8_t debugBuff[80];
void SerialDebug(char * data);
#endif

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_CS_Pin GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOA
#define TPS2373_TPH_Pin GPIO_PIN_9
#define TPS2373_TPH_GPIO_Port GPIOD
#define TPS2373_TPL_Pin GPIO_PIN_10
#define TPS2373_TPL_GPIO_Port GPIOD
#define TPS2373_BT_Pin GPIO_PIN_11
#define TPS2373_BT_GPIO_Port GPIOD
#define Red_Pin GPIO_PIN_12
#define Red_GPIO_Port GPIOD
#define Amber_Pin GPIO_PIN_13
#define Amber_GPIO_Port GPIOD
#define Green_Pin GPIO_PIN_14
#define Green_GPIO_Port GPIOD
#define Orange_Pin GPIO_PIN_15
#define Orange_GPIO_Port GPIOD
#define TPS_OSS_Pin GPIO_PIN_6
#define TPS_OSS_GPIO_Port GPIOC
#define Reset_Pin GPIO_PIN_8
#define Reset_GPIO_Port GPIOC
#define KSZ_INT_Pin GPIO_PIN_9
#define KSZ_INT_GPIO_Port GPIOC
#define KSZ_INT_EXTI_IRQn EXTI9_5_IRQn
#define LVTTL_CLK_Pin GPIO_PIN_9
#define LVTTL_CLK_GPIO_Port GPIOA
#define LVTTL_SYNC_Pin GPIO_PIN_10
#define LVTTL_SYNC_GPIO_Port GPIOA
#define TPS_RST_Pin GPIO_PIN_11
#define TPS_RST_GPIO_Port GPIOA
#define TPS_INT_Pin GPIO_PIN_12
#define TPS_INT_GPIO_Port GPIOA
#define TPS_INT_EXTI_IRQn EXTI15_10_IRQn
#define TPS_I2C_SCL_Pin GPIO_PIN_6
#define TPS_I2C_SCL_GPIO_Port GPIOB
#define TPS_I2C_SDA_Pin GPIO_PIN_7
#define TPS_I2C_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define  FIRMWARE_VERSION  "V1.0.0"         // For Firmware Version
#define  FIRMWARE_SIZE     194              // For Firmware size in KB
#define  HSPI              &hspi3           // Macro For SPI
#define  HI2C              &hi2c1           // Macro For I2C

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
