/*
 ====================================================================================================
 * File:        save.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 05, 2023
 * Brief:       his file provides flash write and read functionality.
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


#ifndef INC_SAVE_H_
#define INC_SAVE_H_

/* Incldes */
#include "stm32f4xx_hal.h"
#include <string.h>

/* Macros */
#define FLASH_STORAGE_ADDRESS     0x080E0000
#define FLASH_ACL_STORAGE_ADDRESS 0x08010000

/* Structures & Enums */
typedef struct __attribute__((packed, aligned(4))) {

    char speed[5][11];                      // Port Speeds
    char status[5][11];                     // Port Status
    char pdconfig[5][11];                   // Port Power Class
    char pdenable[5][11];                   // Port Power Enable
    int  portACL[6][2];                     // Port ACl Mode & Service
    char username[10];                      // Device Login User Name
    char password[10];                      // Device Login Password
    char def[10];                           // Device Setting Status Flag
    char boot[10];                          // Device Boot Status
    char fV[10];                            // Device Firmware Version
    char update[10];                        // Device Update Status Flag
    int  size;                              // Device Firmware Size

} DeviceSettings;

typedef enum {
	POE_OK  = 0,                            // Success No Error
	POE_ERR = 1,                            // Fail Error
	POE_WR  = 2,                            // Fail Wrong Input
	POE_UN  = 3,                            // Fail Unknown
	POE_RER = 255                           // Fail To Read Register 

}poe_err;

/* Function Prototypes */ 
poe_err writeSettingsToFlash(DeviceSettings* settings);
void readSettingsFromFlash(DeviceSettings* settings);
poe_err eraseFlashSector(uint32_t sector);
poe_err writeFlash(uint32_t start_address, uint8_t *data, size_t length);
void readFlash(uint32_t start_address, uint8_t *data, size_t length);
poe_err writeACLSheetToFlash(void* sheet, size_t size);
void readACLSheetToFlash(void* sheet, size_t size);

#endif /* INC_SAVE_H_ */
