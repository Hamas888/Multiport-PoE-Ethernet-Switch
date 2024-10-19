/*
 ====================================================================================================
 * File:        server.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 05, 2023
 * Brief:       This file contains the HTTP Server side for POE switch.
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

#ifndef INC_SERVER_H_
#define INC_SERVER_H_

#include "main.h"
#include "lwip/apps/httpd.h"
#include "lwip/tcp.h"

/* Macros */
#define CRC32_POLYNOMIAL          0xEDB88320         // Constant For CRC Calculation
#define FLASH_SECTOR_9_START_ADDR 0x080A0000		 // Sector 9 start address


/* Function Prototypes */
void http_server_init (void);
const char *CGILogin_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *CGISetting_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *CGIUpdatePage_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *CGIUpdateRequest_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *CGIReset_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
void replaceSubstring(char *str, const char *find, const char *replace);
poe_err parseSwitchSetting();
poe_err setPortState(char param[][11]);
poe_err setPortSpeed(char param[][11]);
poe_err setPortPower(char param1[][11], char param2[][11]);
char* portState(int port);
char* portSpeed(int port);
char* portClass(int port);
void extractHeaderValue(const char *header, const char *fieldName, char *value, size_t maxSize);
uint32_t calculateCRC32(const uint8_t *data, size_t length);
uint8_t compareVersions(const char *version1, const char *version2);
poe_err parseACLEntries(char *data);
poe_err parseVLAN(char *data);

#endif /* INC_SERVER_H_ */
