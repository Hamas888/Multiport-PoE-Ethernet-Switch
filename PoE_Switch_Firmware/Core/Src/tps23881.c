/*
 ====================================================================================================
 * File:		tps23881.h
 * Author:		AMS-IOT
 * Version:	    Rev_1.0.0
 * Date:		Dec 05, 2023
 * Brief:		This file contains the driver for TI TPS23881 POE SOC
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

#include "tps23881.h"

uint8_t firmwareVer;				                           // Stores TPS23881 Firmware Version

uint8_t tpsPortsStatusBuffer[5] = {0, 0, 0, 0, 0};             // Buffer For Each Port Connection Status
uint8_t tpsPortsDetectionBuffer[5] = {0, 0, 0, 0, 0};		   // Buffer For Each Port Detection Status
uint8_t tpsPortsClassificationBuffer[5] = {0, 0, 0, 0, 0};	   // Buffer For Each Port Class Status
uint8_t tpsPortsPowerStatus[5] = {0, 0, 0, 0, 0};			   // Buffer For Each Port Power Enable Status
float tpsPortscurrent[5] = {0, 0, 0, 0, 0};					   // Buffer For Each Port Current
float tpsPortsVoltage[5] = {0, 0, 0, 0, 0};					   // Buffer For Each Port Voltage


/**
 * @brief TPS23881 POE initialization
 * @param[in]  dev pointer to device struct
 * @param[in] I2C handler
 * @param[in] Operating Mode
 * @return error code
 **/

poe_err TPS23881_init(TPS23881 *dev, I2C_HandleTypeDef *i2chandle, TPS238x_Operating_Modes_t opMode)
{
	/* Delay the SRAM and parity programming at least 50 ms from the initial
	power on (P.1) How to Load TPS2388x SRAM and Parity Code Over I2C (Rev. E) */
	HAL_Delay(100);

	//	Set structure parameters
	dev->i2chandle		= i2chandle;
	dev->initStatus		= 0;


	//	error count
	uint8_t errNum = 0;

	uint8_t regData;
	TPS2381_Interrupt_Mask_Register_t intMask;

	//Status of i2c operation
	poe_err ret;

	//	Check device firmware version
	ret = TPS23881_ReadRegister(dev, TPS23881_FIRMWARE_REVISION_COMMAND, &firmwareVer, DEVICE_ADDR_A);
	if(ret != POE_OK)
	{
		errNum++;
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Failed To Read TPS23881 Firmware Ver\r\n") );
#endif
	}
	else
	{
		if(firmwareVer != SRAM_VERSION)
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------Firmware Ver Mismatch Uploading correct Ver\r\n") );
#endif

			if(firmwareVer == SAFE_MODE)
			{
				ret = TPS23881_UpdateSRAMCodeSafe(dev);
			}
			else
			{
				ret = TPS23881_UpdateSRAMCode(dev);
			}
		}
		else
		{
			dev->initStatus = 1;
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------TPS23881 Correct Firmware Ver \r\n\n") );
#endif
		}

		// Enable/Disable Interrupts by setting 2 INTERRUPT MASK Register (P.41) Datasheet
		intMask.CLMSK_Classificiation_Cycle_Unmask = 1;
		intMask.DEMSK_Detection_Cycle_Unmask = 0;
		intMask.DIMSK_Disconnect_Unmask = 1;
		intMask.PGMSK_Power_Good_Unmask = 1;
		intMask.PEMSK_Power_Enable_Unmask = 1;
		intMask.IFMSK_IFAULT_Unmask = 1;
		intMask.INMSK_Inrush_Fault_Unmask = 1;
		intMask.SUMSK_Supply_Event_Fault_Unmask = 1;

		//	Configure device interrupt
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "Configure Interrupts:-\r\n") );
#endif
		regData = *(uint8_t*)&intMask;
		ret = TPS23881_WriteRegister(dev, TPS23881_INTERRUPT_MASK_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Set interrupt DEV_A ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Set interrupt DEV_A SUC\r\n") );
#endif
		ret = TPS23881_WriteRegister(dev, TPS23881_INTERRUPT_MASK_COMMAND, &regData, DEVICE_ADDR_B);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Set interrupt DEV_B ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Set interrupt DEV_B SUC\r\n\n") );
#endif
/*
		//	Set 2 pair ports in 2 pair 30W mode
		SerialDebug( strcpy((char*) debugBuff, "Set 2 pair ports in 2 pair 30W mode:- \r\n") );

		regData = 0x33;
		ret = TPS23881_WriteRegister(dev, TPS2381_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND, &regData, DEVICE_ADDR_A);
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--30W mode DEV_A ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "30W mode DEV_A SUC\r\n\n") );

		//	Set  4 pair ports in 4 pair 90W mode
		SerialDebug( strcpy((char*) debugBuff, "Set 4 pair ports in 2 pair 90W mode:- \r\n") );

		regData = 0xFF;
		ret = TPS23881_WriteRegister(dev, TPS2381_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND, &regData, DEVICE_ADDR_B);
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--90W mode DEV_B ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--90W mode DEV_B SUC\r\n\n") );
*/
		//	Set 2 pair ports in semi auto mode
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "Set operation mode to Semi Auto:- \r\n") );
#endif
		regData = 0xAA;
		ret = TPS23881_WriteRegister(dev, TPS23881_OPERATING_MODE_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Semi mode DEV_A ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Semi mode DEV_A SUC\r\n") );
#endif
		ret = TPS23881_WriteRegister(dev, TPS23881_OPERATING_MODE_COMMAND, &regData, DEVICE_ADDR_B);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Semi mode DEV_B ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Semi mode DEV_B SUC\r\n\n") );
#endif
		//	Enable all channel's DC disconnect
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "Enable all channel's DC disconnect:- \r\n") );
#endif
		regData = 0x0F;
		ret = TPS23881_WriteRegister(dev, TPS23881_DISCONNECT_ENABLE_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--DC disconnect DEV_A ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--DC disconnect DEV_A SUC\r\n") );
#endif
		ret = TPS23881_WriteRegister(dev, TPS23881_DISCONNECT_ENABLE_COMMAND, &regData, DEVICE_ADDR_B);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--DC disconnect DEV_B ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--DC disconnect DEV_B SUC\r\n\n") );
#endif
		//	Power off all ports in case we are re-running this application without physically shutting down ports from previous run
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "Power off all ports:- \r\n") );
#endif
		regData = 0xF0;
		ret = TPS23881_WriteRegister(dev, TPS23881_POWER_ENABLE_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Power off DEV_A ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Power off DEV_A SUC\r\n") );
#endif
		ret = TPS23881_WriteRegister(dev, TPS23881_POWER_ENABLE_COMMAND, &regData, DEVICE_ADDR_B);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Power off DEV_B ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Power off DEV_B SUC\r\n\n") );
#endif
		//	Enable all channels' detection and classification
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "Enable all detection and classification:- \r\n") );
#endif
		regData = 0xFF;
		ret = TPS23881_WriteRegister(dev, TPS23881_DETECT_CLASS_ENABLE_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Detection DEV_A ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Detection DEV_A SUC\r\n") );
#endif
		ret = TPS23881_WriteRegister(dev, TPS23881_DETECT_CLASS_ENABLE_COMMAND, &regData, DEVICE_ADDR_B);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "--Detection DEV_B ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "--Detection DEV_B SUC\r\n\n") );
#endif
	}

	return ret;

}

/**
 * @brief Update TPS23881 SRAM Firmware
 * @param[in]  dev pointer to device struct
 * @return  error code
 **/

uint16_t SRAM_index = 0;

poe_err TPS23881_UpdateSRAMCode(TPS23881 *dev)
{
#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "SRAM_UPLOAD\r\n") );
#endif
	poe_err ret;
	uint8_t regData;
	SRAM_index = 0;

	regData = 0x01;
	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x01 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x01 SUC\r\n") );
#endif

	regData = 0x00;
	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_LSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif

	regData = 0x80;
	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_MSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x80 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x80 SUC\r\n") );
#endif
	if(PARITY_EN == 1)
	{
		regData = 0xC4;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0xC4 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0xC4 SUC\r\n") );
#endif
		regData = 0xBC;
		ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_1, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0xBC ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0xBC SUC\r\n") );
#endif
		regData = 0x02;
		ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_2, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x02 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x02 SUC\r\n") );
#endif
		regData = 0x00;
		ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_3, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
		regData = 0x00;
		ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_4, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
		regData = 0x00;
		ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_2, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
		regData = 0x00;
		ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_1, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
		for(SRAM_index = 0; SRAM_index< NUM_PARITY_BYTES; SRAM_index++)
		{
			regData = ParityCode[SRAM_index];
			ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_DATA_COMMAND, &regData, DEVICE_ADDR_A);
			if(ret != POE_OK)
			{
#ifdef SERIAL_DEBUG
				SerialDebug( strcpy((char*) debugBuff, "ParityCode ERR\r\n") );
#endif
			}
		}

		regData = 0xC5;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0xC5 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0xC5 SUC\r\n") );
#endif
		regData = 0x00;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_LSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
		regData = 0x80;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_MSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x80 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x80 SUC\r\n") );
#endif
	}

	regData = 0xC0;
	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0xC0 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0xC0 SUC\r\n") );
#endif

	for(SRAM_index = 0; SRAM_index< NUM_SRAM_BYTES; SRAM_index++)
	{
		regData = SRAMCode[SRAM_index];
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_DATA_COMMAND, &regData, DEVICE_ADDR_A);
		if(ret != POE_OK)
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "SRAMCode ERR\r\n") );
#endif
		}
	}

	if(PARITY_EN == 1)
	{
		regData = 0x18;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x18 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x18 SUC\r\n") );
#endif
	}
	else
	{
		regData = 0x08;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x08 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x08 SUC\r\n") );
#endif
	}

	//	Delay for approximately 12 ms (P.2) How to Load TPS2388x SRAM and Parity Code Over I2C (Rev. E)
	HAL_Delay(20);

	//	Check the firmware version to make sure the SRAM code has been loaded properly
	ret = TPS23881_ReadRegister(dev, TPS23881_FIRMWARE_REVISION_COMMAND, &firmwareVer, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "firmeareVer ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "firmeareVer SUC\r\n") );
#endif

	if (firmwareVer == SRAM_VERSION)
	{
		dev->initStatus = 1;
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "SRAM Code Load Complete!\r\n") );
#endif
		return POE_OK;
	}
	else
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "SRAM Code Load Error!\r\n") );
#endif
	}

	return ret;

}

/**
 * @brief  Safe Update TPS23881 SRAM Firmware
 * @param[in]  dev pointer to device struct
 * @return  error code
 **/

poe_err TPS23881_UpdateSRAMCodeSafe(TPS23881 *dev)
{
#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "SRAM_UPLOAD_SAFE_MODE\r\n") );
#endif

	poe_err ret;
	uint8_t regData;
	SRAM_index = 0;

		regData = 0x01;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x01 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x01 SUC\r\n") );
#endif
		regData = 0x00;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_LSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
		regData = 0x80;
		ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_MSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
		ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x80 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x80 SUC\r\n") );
#endif
	    if(PARITY_EN == 1)
	    {
	    	regData = 0x84;
			ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x84 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x84 SUC\r\n") );
#endif
			regData = 0xBC;
			ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_1, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0xBC ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0xBC SUC\r\n") );
#endif
			regData = 0x02;
			ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_2, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x02 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x02 SUC\r\n") );
#endif
			regData = 0x00;
			ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_3, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
			regData = 0x00;
			ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_4, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
			regData = 0x00;
			ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_2, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
			regData = 0x00;
			ret = TPS23881_WriteRegister(dev, TPS23881_RAM_PREP_COMMAND_1, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
			ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
	        for(SRAM_index = 0; SRAM_index< NUM_PARITY_BYTES; SRAM_index++)
	        {
	        	regData = ParityCode[SRAM_index];
	        	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_DATA_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	        	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "ParityCode ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "ParityCode SUC\r\n") );
#endif
	        }

	        regData = 0x85;
	        ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	        ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x85 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x85 SUC\r\n") );
#endif
	        regData = 0x00;
	        ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_LSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	        ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
	        regData = 0x80;
	        ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_START_ADDRESS_MSB_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	        ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x00 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x00 SUC\r\n") );
#endif
	    }

	    regData = 0x80;
	    ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	    ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x80 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x80 SUC\r\n") );
#endif

	    for(SRAM_index = 0; SRAM_index< NUM_SRAM_BYTES; SRAM_index++)
	    {
	    	regData = SRAMCode[SRAM_index];
	    	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_DATA_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	    	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "SRAMCode ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "SRAMCode SUC\r\n") );
#endif
	    }

	    if(PARITY_EN == 1)
	    {
	    	regData = 0x18;
	    	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	    	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x18 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x18 SUC\r\n") );
#endif
	    }
	    else
	    {
	    	regData = 0x08;
	    	ret = TPS23881_WriteRegister(dev, TPS23881_SRAM_CONTROL_COMMAND, &regData, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	    	ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "0x08 ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "0x08 SUC\r\n") );
#endif
	    }

	    //	Delay for approximately 12 ms (P.2) How to Load TPS2388x SRAM and Parity Code Over I2C (Rev. E)
	    HAL_Delay(20);


	    //	Check the firmware version to make sure the SRAM code has been loaded properly
	    ret = TPS23881_ReadRegister(dev, TPS23881_FIRMWARE_REVISION_COMMAND, &firmwareVer, DEVICE_ADDR_A);
#ifdef SERIAL_DEBUG
	    ret != POE_OK ? SerialDebug( strcpy((char*) debugBuff, "firmeareVer ERR\r\n") ) : SerialDebug( strcpy((char*) debugBuff, "firmeareVer SUC\r\n") );
#endif
	    if (firmwareVer == SRAM_VERSION)
	    {
	    	dev->initStatus = 1;
#ifdef SERIAL_DEBUG
	    	SerialDebug( strcpy((char*) debugBuff, "SRAM Code Load Complete!\r\n") );
#endif
	        return POE_OK;
	    }
	    else
	    {
#ifdef SERIAL_DEBUG
	    	SerialDebug( strcpy((char*) debugBuff, "SRAM Code Load Error!\r\n") );
#endif
	    }

	    return ret;
}



/**
 * @brief  Read an amount of data in blocking mode from a specific memory address
 * @param[in]  dev pointer to device struct
 * @param[in]  reg Internal memory address
 * @param[in]  data Pointer to data buffer
 * @param[in]  device address type
 * @return error code
 **/

poe_err TPS23881_ReadRegister(TPS23881 *dev, uint8_t reg, uint8_t *data, uint8_t devAdd)
{
	HAL_StatusTypeDef ret;
	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = HAL_I2C_Mem_Read( dev->i2chandle, TPS23881_I2C_ADDR_1, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TPS23881_I2C_TIMEOUT_DELAY);
			break;
		case DEVICE_ADDR_B:
			ret = HAL_I2C_Mem_Read( dev->i2chandle, TPS23881_I2C_ADDR_2, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TPS23881_I2C_TIMEOUT_DELAY);
			break;
		case DEVICE_ADDR_GLOBAL:
			ret = HAL_I2C_Mem_Read( dev->i2chandle, TPS23881_I2C_ADDR_GLOBAL, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TPS23881_I2C_TIMEOUT_DELAY);
			break;
		default:
			return POE_WR;
	}

	if(ret != HAL_OK)
	{
		return POE_ERR;
	}

	return POE_OK;
}


/**
 * @brief  Write an amount of data in blocking mode from a specific memory address
 * @param[in]  dev pointer to device struct
 * @param[in]  reg Internal memory address
 * @param[in]  data Pointer to data buffer needs to be send
 * @param[in]  device address type
 * @return  error code
 **/

poe_err TPS23881_WriteRegister(TPS23881 *dev, uint8_t reg, uint8_t *data, uint8_t devAdd)
{
	HAL_StatusTypeDef ret;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = HAL_I2C_Mem_Write( dev->i2chandle, TPS23881_I2C_ADDR_1, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TPS23881_I2C_TIMEOUT_DELAY);
			break;
		case DEVICE_ADDR_B:
			ret = HAL_I2C_Mem_Write( dev->i2chandle, TPS23881_I2C_ADDR_2, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TPS23881_I2C_TIMEOUT_DELAY);
			break;
		case DEVICE_ADDR_GLOBAL:
			ret = HAL_I2C_Mem_Write( dev->i2chandle, TPS23881_I2C_ADDR_GLOBAL, reg, I2C_MEMADD_SIZE_8BIT, data, 1, TPS23881_I2C_TIMEOUT_DELAY);
		default:
			return POE_WR;
	}

	if(ret != HAL_OK)
	{
		return POE_ERR;
	}

	return POE_OK;

}

/**
 * @brief  Get the current interrupt status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return	 error code
 **/

poe_err TPS23881_GetDeviceInterruptStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, TPS23881_INTERRUPT_COMMAND, &value, DEVICE_ADDR_A);
			dev->interruptStatusA = *(TPS2381_Interrupt_Register_t*)&value;
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, TPS23881_INTERRUPT_COMMAND, &value, DEVICE_ADDR_B);
			dev->interruptStatusB = *(TPS2381_Interrupt_Register_t*)&value;
			break;

		default:
			return POE_WR;
	}

	return ret;
}

/**
 * @brief  Get and clear the power event status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return	 error code
 **/

poe_err TPS23881_GetAndClearDevicePowerEventStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, TPS23881_POWER_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_A);
			dev->powerEventStatusA = *(TPS2381_Power_Event_Register_t*)&value;
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, TPS23881_POWER_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_B);
			dev->powerEventStatusB = *(TPS2381_Power_Event_Register_t*)&value;
			break;

		default:
			return POE_WR;
	}

	return ret;
}

/**
 * @brief Get and clear the detection cycle event status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return error code
 **/

poe_err TPS23881_GetAndClearDeviceDetectionEventStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, TPS23881_DETECTION_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_A);
			dev->detectionEventStatusA = *(TPS2381_Detection_Event_Register_t*)&value;
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, TPS23881_DETECTION_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_B);
			dev->detectionEventStatusB = *(TPS2381_Detection_Event_Register_t*)&value;
			break;

		default:
			return POE_WR;
	}

	return ret;
}

/**
 * @brief Get and clear the fault event status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return error code
 **/

poe_err TPS23881_GetAndClearDeviceFaultEventStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, TPS23881_FAULT_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_A);
			dev->faultEventStatusA = *(TPS2381_Fault_Event_Register_t*)&value;
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, TPS23881_FAULT_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_B);
			dev->faultEventStatusB = *(TPS2381_Fault_Event_Register_t*)&value;
			break;

		default:
			return POE_WR;
	}

	return ret;
}

/**
 * @brief	Get and clear the Inrush and ILIM fault event status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return	error code
 **/

poe_err TPS23881_GetAndClearDeviceInrushEventStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, TPS23881_START_LIMIT_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_A);
			dev->inrushIlimEventStatusA = *(TPS2381_Inrush_ILIM_Event_Register_t*)&value;
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, TPS23881_START_LIMIT_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_B);
			dev->inrushIlimEventStatusB = *(TPS2381_Inrush_ILIM_Event_Register_t*)&value;
			break;

		default:
			return POE_WR;
	}

	return ret;
}

/**
 * @brief	Get and clear system power supply fault event status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return	error code
 **/

poe_err TPS23881_GetAndClearDevicePowerSypplyEventStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, TPS23881_SUPPLY_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_A);
			dev->supplyEventStatusA = *(TPS2381_Supply_Event_4PPCUT_Register_t*)&value;
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, TPS23881_SUPPLY_EVENT_CLEAR_COMMAND, &value, DEVICE_ADDR_B);
			dev->supplyEventStatusB = *(TPS2381_Supply_Event_4PPCUT_Register_t*)&value;
			break;

		default:
			return POE_WR;


	}
	return ret;
}

/**
 * @brief	Get channel power classification
 * @param[in]  dev pointer to device struct
 * @param[in]  status command
 * @param[in]  device address type
 * @return	error code
 **/

poe_err TPS23881_GetChannelDiscoveryAndClass(TPS23881 *dev, uint8_t statusCommand, uint8_t devAdd)
{
	poe_err ret;
	uint8_t value = 0x00;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_ReadRegister(dev, statusCommand, &value, DEVICE_ADDR_A);

			dev->classificationStatusA = (TPS2381_Classification_Status_t)GET_CLASS(value);
			dev->detectionStatusA = (TPS2381_Detection_Status_t)GET_DETECT(value);
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_ReadRegister(dev, statusCommand, &value, DEVICE_ADDR_B);

			dev->classificationStatusB = (TPS2381_Classification_Status_t)GET_CLASS(value);
			dev->detectionStatusB = (TPS2381_Detection_Status_t)GET_DETECT(value);
			break;

		default:
			return POE_WR;


	}

	return ret;
}

/**
 * @brief	Get all interrupt status
 * @param[in]  dev pointer to device struct
 * @param[in]  device address type
 * @return	error code
 **/

poe_err GetAllInterruptStatus(TPS23881 *dev, uint8_t devAdd)
{
	poe_err ret;

	switch(devAdd)
	{
		case DEVICE_ADDR_A:
			ret = TPS23881_GetAndClearDevicePowerEventStatus(dev, DEVICE_ADDR_A);
			ret = TPS23881_GetAndClearDeviceDetectionEventStatus(dev, DEVICE_ADDR_A);
			ret = TPS23881_GetAndClearDeviceFaultEventStatus(dev, DEVICE_ADDR_A);
			ret = TPS23881_GetAndClearDeviceInrushEventStatus(dev, DEVICE_ADDR_A);
			ret = TPS23881_GetAndClearDevicePowerSypplyEventStatus(dev, DEVICE_ADDR_A);
			break;

		case DEVICE_ADDR_B:
			ret = TPS23881_GetAndClearDevicePowerEventStatus(dev, DEVICE_ADDR_B);
			ret = TPS23881_GetAndClearDeviceDetectionEventStatus(dev, DEVICE_ADDR_B);
			ret = TPS23881_GetAndClearDeviceFaultEventStatus(dev, DEVICE_ADDR_B);
			ret = TPS23881_GetAndClearDeviceInrushEventStatus(dev, DEVICE_ADDR_B);
			ret = TPS23881_GetAndClearDevicePowerSypplyEventStatus(dev, DEVICE_ADDR_B);
			break;

		default:
			ret = POE_WR;
			break;
	}

	return ret;
}

/**
 * @brief	Handles the interrupt
 * @param[in]  dev pointer to device struct
 * @param	Address
 * @return	Port
 **/

uint8_t TPS23881_InterruptHandler(TPS23881 *PSE, uint8_t devAd)
{
	poe_err ret;
	uint8_t port = 0;
	uint8_t statusCommand;
	  //	Read Interrupt register
	  ret = TPS23881_GetDeviceInterruptStatus(PSE, devAd);

	  // Checking Which Port Produced Interrupt
	  if(ret == POE_OK)
	  {
		  if(PSE->interruptStatusA.DETC_Detection_Cycle)
		  {
			  ret = TPS23881_GetAndClearDeviceDetectionEventStatus(PSE, devAd);
		  }

		  if(PSE->detectionEventStatusA.DETC1_Detection_Cycle_Event_Channel_1)
		  {
			  if(PSE->detectionEventStatusA.CLSC1_Classification_Cycle_Event_Channel_1)
			  {
				  port = 1;
				  statusCommand = TPS23881_CHANNEL_1_STATUS_COMMAND;
			  }
		  }
		  else if(PSE->detectionEventStatusA.DETC2_Detection_Cycle_Event_Channel_2)
		  {
			  if(PSE->detectionEventStatusA.CLSC2_Classification_Cycle_Event_Channel_2)
			  {
				  port = 2;
				  statusCommand = TPS23881_CHANNEL_2_STATUS_COMMAND;
			  }
		  }
		  else if(PSE->detectionEventStatusA.DETC3_Detection_Cycle_Event_Channel_3)
		  {
			  if(PSE->detectionEventStatusA.CLSC3_Classification_Cycle_Event_Channel_3)
			  {
				  port = 3;
				  statusCommand = TPS23881_CHANNEL_3_STATUS_COMMAND;
			  }
		  }
		  else if(PSE->detectionEventStatusA.DETC4_Detection_Cycle_Event_Channel_4)
		  {
			  if(PSE->detectionEventStatusA.CLSC4_Classification_Cycle_Event_Channel_4)
			  {
				  port = 4;
				  statusCommand = TPS23881_CHANNEL_4_STATUS_COMMAND;
			  }
		  }

		  ret = GetAllInterruptStatus(PSE, DEVICE_ADDR_A);
		  ret = GetAllInterruptStatus(PSE, DEVICE_ADDR_B);

		  ret = TPS23881_PortDetectStatus(PSE, statusCommand, port, devAd);
		  ret = TPS23881_PortClassStatus(PSE, port);

		  PSE->portConnectionStatus.PORT_1_Connected = tpsPortsStatusBuffer[0];
		  PSE->portConnectionStatus.PORT_2_Connected = tpsPortsStatusBuffer[1];
		  PSE->portConnectionStatus.PORT_3_Connected = tpsPortsStatusBuffer[2];
		  PSE->portConnectionStatus.PORT_4_Connected = tpsPortsStatusBuffer[3];

		  // Returning PD Port
		  return port;
	  }

	  // Returning 0 Mean No PD Connected/Disconnected
	  return 0;
}

/**
 * @brief	Detects the PD and store it
 * @param[in]  dev pointer to device struct
 * @param[in]  Status Command
 * @param[in]  Port
 * @param	Address
 * @return	error code
 **/

poe_err TPS23881_PortDetectStatus(TPS23881 *PSE, uint8_t statusCommand, uint8_t port, uint8_t devAd)
{
	  TPS23881_GetAndClearDevicePowerEventStatus(PSE, devAd);

	  // Get The Detection & Classification Status
	  TPS23881_GetChannelDiscoveryAndClass(PSE, statusCommand, devAd);
#ifdef SERIAL_DEBUG
	  sprintf((char*)debugBuff, "\r\n---------Detection of Port %d \r\n\n", port);
	  SerialDebug((char*)debugBuff);
#endif
	  switch(PSE->detectionStatusA)
	  {
		  case CLASS_UNKNOWN :
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: Unknown \r\n"));
#endif
			  break;
		  case DETECT_SHORT_CIRCUIT :
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: Short circuit \r\n"));
#endif
			  break;
		  case DETECT_RESIST_LOW :
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: Too Low \r\n"));
#endif
			  break;
		  case DETECT_RESIST_VALID:
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: Valid \r\n"));
#endif
			  break;
		  case DETECT_RESIST_HIGH :
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: Too High \r\n"));
#endif
			  break;
		  case DETECT_OPEN_CIRCUIT:
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: Open Circuit \r\n"));
#endif
			  break;
		  case DETECT_MOSFET_FAULT :
#ifdef SERIAL_DEBUG
			  SerialDebug( strcpy((char*) debugBuff, "-- Status: MOSFET fault \r\n"));
#endif
		  default:
			  return POE_ERR;
	  }

	  // Store Detection Station In Respective Buffer
	  tpsPortsDetectionBuffer[port - 1] = PSE->detectionStatusA;
	  return POE_OK;
}

/**
  * @brief	Classifies the port and store it
  * @param	dev pointer to device struct
  * @param	Port
  * @return	error code
  */

poe_err TPS23881_PortClassStatus(TPS23881 *PSE, uint8_t port)
{
      // Checks For Valid Classification
	  if(PSE->classificationStatusA != CLASS_OVERCURRENT && PSE->classificationStatusA != CLASS_UNKNOWN && PSE->classificationStatusA != CLASS_MISMATCH)
	  {
		  // PD Connected On Port
		  tpsPortsStatusBuffer[port - 1] = 1;

		  tpsPortsClassificationBuffer[port - 1] = PSE->classificationStatusA;
#ifdef SERIAL_DEBUG
		  sprintf((char*)debugBuff, "\r\n---------Classification of Port %d \r\n\n", port);
		  SerialDebug((char*)debugBuff);
#endif
		  switch(PSE->classificationStatusA)
		  {
			  case CLASS_UNKNOWN:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Unknown \r\n\n"));
#endif
				  break;
			  case CLASS_1:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 1 \r\n\n"));
#endif
				  break;
			  case CLASS_2:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 2 \r\n\n"));
#endif
				  break;
			  case CLASS_3:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 3 \r\n\n"));
#endif
				  break;
			  case CLASS_4:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 4 \r\n\n"));
#endif
				  break;
			  case CLASS_0 :
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 0 \r\n\n"));
#endif
				  break;
			  case CLASS_OVERCURRENT :
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Over current \r\n\n"));
#endif
				  break;
			  case CLASS_5_4P_SINGLE:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 5, 4 Pair Single Signature \r\n\n"));
#endif
				  break;
			  case CLASS_6_4P_SINGLE:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 6, 4 Pair Single Signature \r\n\n"));
#endif
				  break;
			  case CLASS_7_4P_SINGLE:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 7, 4 Pair Single Signature \r\n\n"));
#endif
				  break;
			  case CLASS_8_4P_SINGLE:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 8, 4 Pair Single Signature \r\n\n"));
#endif
				  break;
			  case CLASS_4PLUS_TYPE1 :
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 4+, Type 1 Limited \r\n\n"));
#endif
				  break;
#ifdef SERIAL_DEBUG
			  case CLASS5_4P_DUAL :
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class 5, 4 Pair Dual Signature \r\n\n"));
#endif
				  break;
			  case CLASS_MISMATCH :
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Status: Class Mismatch \r\n\n"));
#endif
				  break;
			  default:
				  return POE_ERR;
		  }
#ifdef SERIAL_DEBUG
		  sprintf((char*)debugBuff, "\r\n---------Port %d Connected\r\n\n", port);
		  SerialDebug((char*)debugBuff);
#endif
		  return POE_OK;
	  }
	  // Otherwise In Valid Classification
	  else
	  {
		  //PD Disconnected On Port
		  tpsPortsStatusBuffer[port - 1] = 0;
		  tpsPortsClassificationBuffer[port - 1] = PSE->classificationStatusA;
#ifdef SERIAL_DEBUG
		  sprintf((char*)debugBuff, "\r\n---------Port %d Disconnected\r\n\n", port);
		  SerialDebug((char*)debugBuff);
#endif
	  }

	  return POE_OK;
}

/**
  * @brief	Enables & Disable the Power
  * @param	dev pointer to device struct
  * @param	Port
  * @param	Power Enable
  * @param	Address
  * @return	error code
  */

poe_err TPS23881_PowerEnable(TPS23881 *PSE, uint8_t port, uint8_t enable, uint8_t devAd)
{
	if(port > 0 && port <= 4)
	{
		poe_err ret;
		uint8_t value;
		TPS23881PortPowerStatus_t channelsStatus;
		TPS23881PortPowerEnable_t channelsPower;

		ret = TPS23881_ReadRegister(PSE, TPS23881_POWER_STATUS_COMMAND, &value, devAd);
		channelsStatus = *(TPS23881PortPowerStatus_t*)&value;

		// Preparing Buffer For Power Enable Command
		channelsPower.Channel_1_Power_ON = channelsStatus.Channel_1_Power_Status;
		channelsPower.Channel_2_Power_ON = channelsStatus.Channel_2_Power_Status;
		channelsPower.Channel_3_Power_ON = channelsStatus.Channel_3_Power_Status;
		channelsPower.Channel_4_Power_ON = channelsStatus.Channel_4_Power_Status;
		channelsPower.Channel_1_Power_OFF = !channelsStatus.Channel_1_Power_Status;
		channelsPower.Channel_2_Power_OFF = !channelsStatus.Channel_2_Power_Status;
		channelsPower.Channel_3_Power_OFF = !channelsStatus.Channel_3_Power_Status;
		channelsPower.Channel_4_Power_OFF = !channelsStatus.Channel_4_Power_Status;

		// If Enable Turning Power On
		if(enable)
		{
			switch(port)
			{
			  case 1:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 1 Power On \r\n\n"));
#endif
				  channelsPower.Channel_1_Power_ON = 1;
				  channelsPower.Channel_1_Power_OFF = 0;
				  break;
			  case 2:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 2 Power On \r\n\n"));
#endif
				  channelsPower.Channel_2_Power_ON = 1;
				  channelsPower.Channel_2_Power_OFF = 0;
				  break;
			  case 3:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 3 Power On \r\n\n"));
#endif
				  channelsPower.Channel_3_Power_ON = 1;
				  channelsPower.Channel_3_Power_OFF = 0;
				  break;
			  case 4:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 4 Power On \r\n\n"));
#endif
				  channelsPower.Channel_4_Power_ON = 1;
				  channelsPower.Channel_4_Power_OFF = 0;
				  break;
			  default:
				  break;
			}
		}
		// Otherwise Turning Power off
		else
		{
			switch(port)
			{
			  case 1:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 1 Off \r\n\n"));
#endif
				  channelsPower.Channel_1_Power_ON = 0;
				  channelsPower.Channel_1_Power_OFF = 1;
				  break;
			  case 2:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 2 Off \r\n\n"));
#endif
				  channelsPower.Channel_2_Power_ON = 0;
				  channelsPower.Channel_2_Power_OFF = 1;
				  break;
			  case 3:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 3 Off \r\n\n"));
#endif
				  channelsPower.Channel_3_Power_ON = 0;
				  channelsPower.Channel_3_Power_OFF = 1;
				  break;
			  case 4:
#ifdef SERIAL_DEBUG
				  SerialDebug( strcpy((char*) debugBuff, "-- Port 4 Off \r\n\n"));
#endif
				  channelsPower.Channel_4_Power_ON = 0;
				  channelsPower.Channel_4_Power_OFF = 1;
				  break;
			  default:
				  break;
			}
		}
		value = *(uint8_t*)&channelsPower;
		ret = TPS23881_WriteRegister(PSE, TPS23881_POWER_ENABLE_COMMAND, &value , devAd);

		// Returning Error
		return ret;
	}

	return POE_WR;
}

/**
  * @brief	Reads & Stores the Power Enable Status
  * @param	dev pointer to device struct
  * @param	Address
  * @return	error code
  */

poe_err TPS23881_PowerStatus(TPS23881 *PSE, uint8_t devAd)
{
		poe_err ret;
		uint8_t value;
		TPS23881PortPowerStatus_t channelsStatus;

		// Fetching Power Status For Ports
		ret = TPS23881_ReadRegister(PSE, TPS23881_POWER_STATUS_COMMAND, &value, devAd);

		if(ret == POE_OK)
		{
			channelsStatus = *(TPS23881PortPowerStatus_t*)&value;

			// Updating The Buffer
			tpsPortsPowerStatus[0] = channelsStatus.Channel_1_Power_Status;
			tpsPortsPowerStatus[1] = channelsStatus.Channel_2_Power_Status;
			tpsPortsPowerStatus[2] = channelsStatus.Channel_3_Power_Status;
			tpsPortsPowerStatus[3] = channelsStatus.Channel_4_Power_Status;

			// Returning Error
			return ret;
		}

	return POE_ERR;
}

/**
  * @brief	Sets the class
  * @param	TPS23881 Structure
  * @param	Class
  * @return	Class Configuration
  */

uint8_t TPS23881_SetPortClass(TPS23881 *PSE, uint8_t class)
{
	TPS23881PortPowerAllocation_t powerClass;
	powerClass._4PWnn = 1;

	// Configuration For Power Class's
	if(class < CLASS_4 || class == CLASS_0)          //Power 15.4 W
	{
		powerClass.MCnn_0 = 0;
		powerClass.MCnn_1 = 0;
		powerClass.MCnn_2 = 0;
	}
	else if(class == CLASS_4)     			         //Power 30 W
	{
		powerClass.MCnn_0 = 1;
		powerClass.MCnn_1 = 1;
		powerClass.MCnn_2 = 0;
	}
	else if(class == CLASS_5_4P_SINGLE)              //Power 45 W
	{
		powerClass.MCnn_0 = 0;
		powerClass.MCnn_1 = 0;
		powerClass.MCnn_2 = 1;
	}
	else if(class == CLASS_6_4P_SINGLE)              //Power 60 W
	{
		powerClass.MCnn_0 = 1;
		powerClass.MCnn_1 = 0;
		powerClass.MCnn_2 = 1;
	}
	else if(class == CLASS_7_4P_SINGLE)              //Power 75 W
	{
		powerClass.MCnn_0 = 0;
		powerClass.MCnn_1 = 1;
		powerClass.MCnn_2 = 1;
	}
	else                                            //Power 90 W
	{
		powerClass.MCnn_0 = 1;
		powerClass.MCnn_1 = 1;
		powerClass.MCnn_2 = 1;
	}

	// Returning Configuration
	return (*(uint8_t*)&powerClass) & 0x0F;
}

/**
  * @brief	Assign Power Class to the Port
  * @param	TPS23881 Structure
  * @param	Port
  * @param	Class
  * @param	Address
  * @return	error code
  * Note: The CLass Can Be Assigned On Port Pair Base's Mean 1/2 Or 3/4
  */

poe_err TPS23881_AssignPortClass(TPS23881 *PSE, uint8_t port, uint8_t class, uint8_t devAd)
{
	poe_err ret;
	uint8_t value;
	uint8_t temp;

	ret = TPS23881_PowerStatus(PSE, devAd);

	// Assigning Class For Port 1/2
	if(port < 3 && port <= 0)
	{
		// Disabling Power For 1/2 Before Assigning Class
		ret = TPS23881_PowerEnable(PSE, 1, Disable, devAd);
		ret = TPS23881_PowerEnable(PSE, 2, Disable, devAd);

		// Get The Class Configuration
		temp = TPS23881_SetPortClass(PSE, class);

		ret = TPS23881_ReadRegister(PSE, TPS23881_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND, &value, devAd);

		value = (value & 0xF0) | temp;

		// Setting The Class For 1/2
		ret = TPS23881_WriteRegister(PSE, TPS23881_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND, &value, devAd);

		// Resuming Ports Power Status
		ret = TPS23881_PowerEnable(PSE, 1, tpsPortsPowerStatus[0], devAd);
		ret = TPS23881_PowerEnable(PSE, 2, tpsPortsPowerStatus[1], devAd);

		// Returning Error
		return ret;

	}

	// Assigning Class For Port 3/4
	else if (port > 2 && port <= 5)
	{
		// Disabling Power For 3/4 Before Assigning Class
		ret = TPS23881_PowerEnable(PSE, 3, Disable, devAd);
		ret = TPS23881_PowerEnable(PSE, 4, Disable, devAd);

		temp = TPS23881_SetPortClass(PSE, class);

		// Get The Class Configuration
		ret = TPS23881_ReadRegister(PSE, TPS23881_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND, &value, devAd);

		value = (value & 0x0F) | (temp << 4);

		// Setting The Class For 3/4
		ret = TPS23881_WriteRegister(PSE, TPS23881_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND, &value, devAd);

		// Resuming Ports Power Status
		ret = TPS23881_PowerEnable(PSE, 3, tpsPortsPowerStatus[2], devAd);
		ret = TPS23881_PowerEnable(PSE, 4, tpsPortsPowerStatus[3], devAd);

		// Returning Error
		return ret;
	}

	return POE_ERR;
}

/**
  * @brief	Reads & Stores Voltage for Connected Ports
  * @param	TPS23881 Structure
  * @param	Address
  * @return	error code
  */

poe_err TPS23881_ReadPortsCurrents(TPS23881 *PSE, uint8_t devAd)
{
	poe_err ret;
	uint16_t value = 0;
	uint8_t temp;
	uint8_t channelCommand[4] = {	TPS23881_CHANNEL_1_CURRENT_COMMAND,
									TPS23881_CHANNEL_2_CURRENT_COMMAND,
									TPS23881_CHANNEL_3_CURRENT_COMMAND,
									TPS23881_CHANNEL_4_CURRENT_COMMAND	};

	// Reading Currents For On Ports
	for(int port = 1; port <=4; port++)
	{
		if(tpsPortsPowerStatus[port - 1])
		{
			ret = TPS23881_ReadRegister(PSE, channelCommand[port - 1], &temp, devAd);
			value = temp;
			ret = TPS23881_ReadRegister(PSE, channelCommand[port - 1] + 1, &temp, devAd);
			value |= ((uint16_t)temp << 8);
			value &= 0x1FFF;
			tpsPortscurrent[port - 1] = value * I_STEP;
		}
	}

	// Returning Error
	return ret;
}

/**
  * @brief	Reads & Stores Current for Connected Ports
  * @param	TPS23881 Structure
  * @param	Address
  * @return	error code
  */

poe_err TPS23881_ReadPortsVoltages(TPS23881 *PSE, uint8_t devAd)
{
	poe_err ret;
	uint16_t value = 0;
	uint8_t temp;
	uint8_t channelCommand[4] = {	TPS23881_CHANNEL_1_VOLTAGE_COMMAND,
									TPS23881_CHANNEL_2_VOLTAGE_COMMAND,
									TPS23881_CHANNEL_3_VOLTAGE_COMMAND,
									TPS23881_CHANNEL_4_VOLTAGE_COMMAND	};

	// Reading Voltages For On Ports
	for(int port = 1; port <=4; port++)
	{
		if(tpsPortsPowerStatus[port - 1])
		{
			ret = TPS23881_ReadRegister(PSE, channelCommand[port - 1], &temp, devAd);
			value = temp;
			ret = TPS23881_ReadRegister(PSE, channelCommand[port - 1] + 1, &temp, devAd);
			value |= ((uint16_t)temp << 8);
			value &= 0x1FFF;
			tpsPortsVoltage[port - 1] = value * V_STEP;
		}
	}

	// Returning Error
	return ret;
}

/**
  * @brief	Calculate Power For Connected Ports & Returns The Abnormal Port
  * @param	TPS23881 Structure
  * @param	Address
  * @return	Port
  */

uint8_t TPS23881_PortAutoRecovery(TPS23881 *PSE, uint8_t devAd)
{
	float power;

	// Getting Current and Voltage For Ports
	TPS23881_ReadPortsVoltages(PSE, devAd);
	TPS23881_ReadPortsCurrents(PSE, devAd);

	// Loop Through Ports
	for(int port = 1; port <=4; port++)
	{
		// Checks If Power Is On For Port
		if(tpsPortsPowerStatus[port - 1])
		{
			// Calculates Power Consumed By The PD On Port
			power = tpsPortscurrent[port - 1] * tpsPortsVoltage[port - 1];

			// Checks If Power Consumed Is Within Range Of Port Power Class & Return Port With Issue
			switch(tpsPortsClassificationBuffer[port - 1])
			{
			  case CLASS_1:
				  if((power > 4) || (power < 3))
				  {
					  return port;
				  }
				  break;
			  case CLASS_2:
				  if((power > 7) || (power < 6))
				  {
					  return port;
				  }
				  break;
			  case CLASS_3:
				  if((power > 15.5) || (power < 12))
				  {
					  return port;
				  }
				  break;
			  case CLASS_4:
				  if((power > 30) || (power < 25))
				  {
					  return port;
				  }
				  break;
			  case CLASS_0 :
				  if((power > 15.5) || (power < 12))
				  {
					  return port;
				  }
				  break;
			  case CLASS_5_4P_SINGLE:
				  if((power > 45) || (power < 40))
				  {
					  return port;
				  }
				  break;
			  case CLASS_6_4P_SINGLE:
				  if((power > 60) || (power < 51))
				  {
					  return port;
				  }
				  break;
			  case CLASS_7_4P_SINGLE:
				  if((power > 75) || (power < 62))
				  {
					  return port;
				  }
				  break;
			  case CLASS_8_4P_SINGLE:
				  if((power > 90) || (power < 73))
				  {
					  return port;
				  }
				  break;
			  case CLASS_4PLUS_TYPE1 :
				  if((power > 90) || (power < 60))
				  {
					  return port;
				  }
				  break;
			  case CLASS5_4P_DUAL :
				  if((power > 90) || (power < 75))
				  {
					  return port;
				  }
				  break;
			  default:
				  break;
			}
		}
	}

	// Returning 0 Mean No Issue
	return 0;
}

/**
  * @brief	Restart Detection & Classification
  * @param	TPS23881 Structure
  * @param	Port
  * @param	Address
  * @return	error code
  */

poe_err TPS23881_RestartDetectClass(TPS23881 *PSE, uint8_t port, uint8_t devAd)
{
	TPS2381_Detection_Event_Register_t restart = *(TPS2381_Detection_Event_Register_t*)0;
	poe_err ret;
	uint8_t value = 0;

	// Setting Detection & Classification Bits For Port
	switch(port)
	{
		case 1:
			restart.DETC1_Detection_Cycle_Event_Channel_1 = 1;
			restart.CLSC1_Classification_Cycle_Event_Channel_1 = 1;
			break;
		case 2:
			restart.DETC2_Detection_Cycle_Event_Channel_2 = 1;
			restart.CLSC2_Classification_Cycle_Event_Channel_2 = 1;
			break;
		case 3:
			restart.DETC3_Detection_Cycle_Event_Channel_3 = 1;
			restart.CLSC3_Classification_Cycle_Event_Channel_3 = 1;
			break;
		case 4:
			restart.DETC4_Detection_Cycle_Event_Channel_4 = 1;
			restart.CLSC4_Classification_Cycle_Event_Channel_4 = 1;
			break;
		default:
			break;
	}

	// Restarting Detection & Classification
	value = *(uint8_t*)&restart;
	ret = TPS23881_WriteRegister(PSE, TPS23881_DETECT_CLASS_RESTART_COMMAND, &value, devAd);

	// Returning Error
	return ret;
}
