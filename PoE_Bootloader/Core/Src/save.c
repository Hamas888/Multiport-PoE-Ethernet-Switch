/*
 ====================================================================================================
 * File:		save.h
 * Author:		AMS-IOT
 * Version:	    Rev_1.0.0
 * Date:		Jan 5, 2024
 * Brief:		This file contains functionality to write and read flash
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
 ====================================================================================================
 */

#include "save.h"
#include "main.h"

/**
 * @brief Write settings to Flash memory
 * @param[in] DeviceSettings structure
 * @return  error code
 **/

poe_err writeSettingsToFlash(DeviceSettings* settings)
{
	if(eraseFlashSector(FLASH_SECTOR_11) != POE_OK) {
		return POE_ERR;
	}

    HAL_FLASH_Unlock();

    // Write settings to Flash memory
    uint32_t* data = (uint32_t*)settings;

    for (int i = 0; i < sizeof(DeviceSettings) / sizeof(uint32_t); ++i) {

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORAGE_ADDRESS + (i * 4), data[i]) != HAL_OK) {
            // Error handling
#ifdef  SERIAL_DEBUG
            SerialDebug( strcpy((char*) debugBuff, "\n---------Error while writing settings to flash\r\n"));
#endif
            HAL_FLASH_Lock();

            return POE_ERR;
        }
    }

    HAL_FLASH_Lock();

    return POE_OK;
}


/**
 * @brief Read settings from flash memory
 * @param[in] DeviceSettings structure
 **/

void readSettingsFromFlash(DeviceSettings* settings)
{
	// Copy data from Flash memory to the structure
	memcpy(settings, (DeviceSettings*)FLASH_STORAGE_ADDRESS, sizeof(DeviceSettings));
}

/**
 * @brief Erase specific flash sector
 * @param[in] Address of the Sector
 * @return  error code
 **/

poe_err eraseFlashSector(uint32_t sector)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInitStruct;
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInitStruct.Sector = sector;
    eraseInitStruct.NbSectors = 1;

    uint32_t sectorError = 0;

    if (HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK) {
        // Error handling
#ifdef  SERIAL_DEBUG
        SerialDebug( strcpy((char*) debugBuff, "\n---------Error while erasing flash sector\r\n"));
#endif
        HAL_FLASH_Lock();

        return POE_ERR;
    }

    HAL_FLASH_Lock();

    return POE_OK;
}

/**
 * @brief Write data to flash
 * @param[in] Start Address
 * @param[in] Data
 * @param[in] Size of the data
 * @return  error code
 **/

poe_err writeFlash(uint32_t start_address, uint8_t *data, size_t length)
{
    HAL_FLASH_Unlock();

    // Write data to flash
    for (size_t i = 0; i < length; i += 4) {
        uint32_t data_word;

        memcpy(&data_word, &data[i], sizeof(uint32_t));

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, start_address, data_word) != HAL_OK)  {
            // Error handling
#ifdef  SERIAL_DEBUG
            SerialDebug( strcpy((char*) debugBuff, "\n---------Error while writing data to flash\r\n"));
#endif
            HAL_FLASH_Lock();

            return POE_ERR;
        }

        start_address += sizeof(uint32_t);
    }

    HAL_FLASH_Lock();

    return POE_OK;
}

/**
 * @brief Read data from flash
 * @param[in] Start Address
 * @param[in] Data
 * @param[in] Size of the data
 **/

void readFlash(uint32_t start_address, uint8_t *data, size_t length)
{
    // Read data from flash
    for (size_t i = 0; i < length; i += 4) {
        uint32_t data_word = *(__IO uint32_t*)start_address;

        memcpy(&data[i], &data_word, sizeof(uint32_t));

        start_address += sizeof(uint32_t);
    }
}

/**
 * @brief Write ACL sheet to Flash memory
 * @param[in] structure
 * @param[in] structure size
 * @return  error code
 **/

poe_err writeACLSheetToFlash(void* sheet, size_t size)
{
	if(eraseFlashSector(FLASH_SECTOR_4) != POE_OK) {
		return POE_ERR;
	}

    HAL_FLASH_Unlock();

    // Write sheet to Flash memory
    uint32_t* data = (uint32_t*)sheet;

    for (int i = 0; i < size / sizeof(uint32_t); ++i) {

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_ACL_STORAGE_ADDRESS + (i * 4), data[i]) != HAL_OK) {
            // Error handling
#ifdef  SERIAL_DEBUG
            SerialDebug( strcpy((char*) debugBuff, "\n---------Error while writing ACL sheet to flash\r\n"));
#endif
            HAL_FLASH_Lock();

            return POE_ERR;
        }
    }

    HAL_FLASH_Lock();

    return POE_OK;
}

/**
 * @brief Read ACL sheet from flash memory
 * @param[in] structure
 * @param[in] structure size
 **/

void readACLSheetToFlash(void* sheet, size_t size)
{
	// Copy data from Flash memory to the structure
	memcpy(sheet, (void*)FLASH_ACL_STORAGE_ADDRESS, size);
}

