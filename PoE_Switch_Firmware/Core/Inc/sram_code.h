/*
 ====================================================================================================
 * File:        sram_code.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 05, 2023
 * Brief:       Holds firmware updates for TI TPS23881 POE SOC.
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

#ifndef INC_SRAM_CODE_H_
#define INC_SRAM_CODE_H_

/* Incldes */
#include "stdint.h"

/* Macros */
#define NUM_SRAM_BYTES                        0x4000
#define NUM_PARITY_BYTES                      0x800
#define SRAM_VERSION                          0x14
#define SAFE_MODE                          	  0xFF

/* Extern Variables */
extern const uint8_t SRAMCode[];
extern const uint8_t ParityCode[];


#endif /* INC_SRAM_CODE_H_ */
