/*
 ====================================================================================================
 * File:		ksz9897.c
 * Author:		AMS-IOT
 * Version:	    Rev_1.0.0
 * Date:		Dec 05, 2023
 * Brief:		This file contains the driver for Microchip KSZ9897 Ethernet switch
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

#include "ksz9897.h"
#include "server.h"

/**
 * @brief Switch port status array
 **/

uint8_t kszPortsStatusBuffer[5] = {Disconnected, Disconnected, Disconnected, Disconnected, Disconnected};

/**
 * @brief KSZ9897 Ethernet switch initialization
 * @param[in] handler SPI handler
 * @param[in] tail tagging feature
 * @return Error code
 **/

poe_err ksz9897Init(SPI_HandleTypeDef *hspi, bool ETH_PORT_TAGGING_SUPPORT)
{
	poe_err ret;
	uint8_t temp = 0;
	uint8_t count = 0;

	// Wait for the serial interface to be ready
	do
	{
		// Read CHIP_ID1 register
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_CHIP_ID1);

		// Trying 10 times to establish communication before quitting
		if(count >= 10 && temp == POE_RER)
		{
			return POE_RER;
		}
        count++;
		// The returned data is invalid until the serial interface is ready
	} while(temp != KSZ9897_CHIP_ID1_DEFAULT);

#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "---------Connection Established with KSZ9897\r\n") );
#endif
	// Soft resetting switch bringing all registers to there default values
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_SWITCH_OP, KSZ9897_SWITCH_OP_SOFT_HARD_RESET);
#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "---------Initializing KSZ9897 with loaded parameters\r\n") );
#endif

	// If Tail Tagging Is Enabled
	if (ETH_PORT_TAGGING_SUPPORT)
	{
		//Enable tail tag feature
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_OP_CTRL0);
		temp |= KSZ9897_PORTn_OP_CTRL0_TAIL_TAG_EN;
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT6_OP_CTRL0, temp);

		//Disable frame length check
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_SWITCH_MAC_CTRL0);
		temp &= ~KSZ9897_SWITCH_MAC_CTRL0_FRAME_LEN_CHECK_EN;
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_SWITCH_MAC_CTRL0, temp);

		// Loop through the ports
		for(int port = KSZ9897_PORT1; port <= KSZ9897_PORT5; port++)
		{
			// Putting ports to listening state
			ksz9897SetPortState(hspi, port, SWITCH_PORT_STATE_LISTENING);

		}
	}
	// If Tail Tagging Disabled
	else
	{
		// Disable tail tag feature
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_OP_CTRL0);
		temp &= ~KSZ9897_PORTn_OP_CTRL0_TAIL_TAG_EN;
		ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT6_OP_CTRL0, temp);
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT7_OP_CTRL0);
		temp &= ~KSZ9897_PORTn_OP_CTRL0_TAIL_TAG_EN;
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT7_OP_CTRL0, temp);

		// Enable frame length check
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_SWITCH_MAC_CTRL0);
		temp |= KSZ9897_SWITCH_MAC_CTRL0_FRAME_LEN_CHECK_EN;
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_SWITCH_MAC_CTRL0, temp);

		// Comment This Loop If You Don't Want To Disable Ethernet On All Port On Boot And Un-Comment The Line Below The This Loop
		for(int port = KSZ9897_PORT1; port <= KSZ9897_PORT5; port++)
		{
			// Disabling Ports
			ksz9897SetPortState(hspi, port, SWITCH_PORT_STATE_DISABLED);
		}

		// If You Want To Set Port State Form Saved Setting Un-Comment This Line
//		setPortState(poeSettings.status);
	}

	// Restore default age count
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_SWITCH_LUE_CTRL0, KSZ9897_SWITCH_LUE_CTRL0_AGE_COUNT_DEFAULT | KSZ9897_SWITCH_LUE_CTRL0_HASH_OPTION_CRC);

	// Restore default age period
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_SWITCH_LUE_CTRL3, KSZ9897_SWITCH_LUE_CTRL3_AGE_PERIOD_DEFAULT);

	// Add internal delay to ingress and egress RGMII clocks
	temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL1);
	temp |= KSZ9897_PORTn_XMII_CTRL1_RGMII_ID_IG;
	temp |= KSZ9897_PORTn_XMII_CTRL1_RGMII_ID_EG;

	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL1, temp);

	temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT7_XMII_CTRL1);
	temp |= KSZ9897_PORTn_XMII_CTRL1_RGMII_ID_IG;
	temp |= KSZ9897_PORTn_XMII_CTRL1_RGMII_ID_EG;

	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT7_XMII_CTRL1, temp);

	// Start switch operation
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_SWITCH_OP,KSZ9897_SWITCH_OP_START_SWITCH);

	// Loop through the ports
	for(int port = KSZ9897_PORT1; port <= KSZ9897_PORT5; port++)
	{
		// Improve PHY receive performance (silicon errata workaround 1)
		ret = ksz9897WriteMmdReg(hspi, port, 0x01, 0x6F, 0xDD0B);
		ret = ksz9897WriteMmdReg(hspi, port, 0x01, 0x8F, 0x6032);
		ret = ksz9897WriteMmdReg(hspi, port, 0x01, 0x9D, 0x248C);
		ret = ksz9897WriteMmdReg(hspi, port, 0x01, 0x75, 0x0060);
		ret = ksz9897WriteMmdReg(hspi, port, 0x01, 0xD3, 0x7777);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x06, 0x3008);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x08, 0x2001);

		// Improve transmit waveform amplitude (silicon errata workaround 2)
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x04, 0x00D0);

		// EEE must be manually disabled (silicon errata workaround 4)
		ret = ksz9897WriteMmdReg(hspi, port, KSZ9897_MMD_EEE_ADV, 0);

		// Adjust power supply settings (silicon errata workaround 7)
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x13, 0x6EFF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x14, 0xE6FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x15, 0x6EFF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x16, 0xE6FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x17, 0x00FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x18, 0x43FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x19, 0xC3FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x1A, 0x6FFF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x1B, 0x07FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x1C, 0x0FFF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x1D, 0xE7FF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x1E, 0xEFFF);
		ret = ksz9897WriteMmdReg(hspi, port, 0x1C, 0x20, 0xEEEE);

		// Select tri-color dual-LED mode (silicon errata workaround 15)
		ret = ksz9897WriteMmdReg(hspi, port, KSZ9897_MMD_LED_MODE, KSZ9897_MMD_LED_MODE_LED_MODE_TRI_COLOR_DUAL | KSZ9897_MMD_LED_MODE_RESERVED_DEFAULT);

	}

	// Setting Switch Max Speed
	ret = ksz9897SettingSpeed(hspi, 100);

	// Enabling link interrupts
	ret = ksz9897EnableInterrupts(hspi);

	// Setting ACL Table for ports


	// Handle Error
	if(ret != POE_OK)
	{

	}

	// Return Error
	return ret;
}

/**
 * @brief Enable Interrupts
 * @param[in] handler SPI handler
 * @return  error code
 **/

poe_err ksz9897EnableInterrupts(SPI_HandleTypeDef *hspi)
{
	poe_err ret;

	for(int port = KSZ9897_PORT1; port <= KSZ9897_PORT5; port++)
	{
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_INT_CTRL(port), KSZ9897_ICSR_LINK_UP_IF | KSZ9897_ICSR_LINK_DOWN_IF);
	}

	// Return Error
	return ret;
}

/**
 * @brief Handle Interrupts
 * @param[in] handler SPI handler
 * @return kszPortsStatusBuffer
 **/

uint8_t* ksz9897InterruptHandler(SPI_HandleTypeDef *hspi)
{
	uint8_t global_port_int_status;
	uint8_t port_int_status;
	// Reading global port status register
	global_port_int_status = ksz9897ReadSwitchReg8(hspi, 0x001B);

	// Checking if there's any interrupt
	if(global_port_int_status > 0)
	{
		for (int i = 6; i >= 0; --i)
		{
			//Checking ports for interrupt
			int bit = (global_port_int_status >> i) & 1;

			if (bit == 1)
			{
				if(i>= 0 && i <= 4)
				{
					// Reading PHY interrupt status register
					port_int_status = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_INT_CTRL_STATUS(i+1));

					// Checking if there's link up interrupt
					if(((port_int_status >> 0) & 1))
					{
						// Filtering any glitches
						if(kszPortsStatusBuffer[i] != Connected)
						{
							// Updating the Port status buffer
							char str[5];
							sprintf(str, "%d", (i + 1));
							kszPortsStatusBuffer[i] = Connected;
#ifdef SERIAL_DEBUG
							SerialDebug( strcpy((char*) debugBuff, "---------Switch Port ") );
							SerialDebug( strcpy((char*) debugBuff, str));
							SerialDebug( strcpy((char*) debugBuff, " Connected \r\n") );
#endif
						}
					}

					// Checking if there's link down interrupt
					else if(((port_int_status >> 2) & 1))
					{
						// Filtering any glitches
						if(kszPortsStatusBuffer[i] != Disconnected)
						{
							// Updating the Port status buffer
							char str[5];
							sprintf(str, "%d", (i + 1));
							kszPortsStatusBuffer[i] = Disconnected;
#ifdef SERIAL_DEBUG
							SerialDebug( strcpy((char*) debugBuff, "---------Switch Port ") );
							SerialDebug( strcpy((char*) debugBuff, str));
							SerialDebug( strcpy((char*) debugBuff, " Disconnected \r\n") );
#endif
						}
					}
				}
			}
		}
	}

	// Returning the port status buffer
	return kszPortsStatusBuffer;
}

/**
 * @brief Set Speed Of All KSZ9897 Ports
 * @param[in] handler SPI handler
 * @return  error code
 **/

poe_err ksz9897SettingSpeed(SPI_HandleTypeDef *hspi, uint8_t speed)
{
	poe_err ret;
	uint8_t temp;

	// Setting Mac port 6 speed 100 Mbps
	temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL0);
	temp |= 0x78;
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL0, temp);

	// Setting port 6 speed 100 Mbps in RGMII Mode
	temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL1);
	temp |= 0x48;
	ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL1, temp);

	// Setting Mac port 7 speed 10 Mbps
	temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT7_XMII_CTRL0);
	temp &= ~ 0x10;
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT7_XMII_CTRL0, temp);

	// Setting port 7 speed 10 Mbps in RMII Mode
	temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT7_XMII_CTRL1);
	temp |= 0x49;
	ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORT7_XMII_CTRL1, temp);

    // Setting PHY Ports Speed Form Saved Settings
	for(int port = KSZ9897_PORT1; port <= KSZ9897_PORT5; port++)
	{
		if(strcmp(poeSettings.speed[port -1], "100 MBps") == 0)
		{
			Ksz9897Portspeed100(hspi, port);
		}
		else if(strcmp(poeSettings.speed[port -1], "10 MBps") == 0)
		{
			ret = Ksz9897Portspeed10(hspi, port);
		}
	}

	// Return Error
	return ret;
}

/**
 * @brief Setting port speed 10Mbps
 * @param[in] handler SPI handler
 * @param[in] port number
 * @return  error code
 **/

poe_err Ksz9897Portspeed10(SPI_HandleTypeDef *hspi, uint8_t port)
{
	poe_err ret;

	ret = ksz9897WriteSwitchReg8(hspi,KSZ9897_PORTn_PHY_CTRL1(port), 0x01);
	ret = ksz9897WriteSwitchReg8(hspi,KSZ9897_PORTn_PHY_CTRL2(port), 0x00);

	// Return Error
	return ret;
}

/**
 * @brief Setting port speed 100Mbps
 * @param[in] handler SPI handler
 * @param[in] port number
 * @return  error code
 **/

poe_err Ksz9897Portspeed100(SPI_HandleTypeDef *hspi, uint8_t port)
{
	poe_err ret;

	ret = ksz9897WriteSwitchReg8(hspi,KSZ9897_PORTn_PHY_CTRL1(port), 0x21);
	ret = ksz9897WriteSwitchReg8(hspi,KSZ9897_PORTn_PHY_CTRL2(port), 0x00);

	// Return Error
	return ret;
}

/**
 * @brief Write switch register (8 bits)
 * @param[in] handler SPI handler
 * @param[in] address Switch register address
 * @param[in] data Register value
 * @return  error code
 **/

poe_err ksz9897WriteSwitchReg8(SPI_HandleTypeDef *hspi, uint16_t address,uint8_t data)
{
	HAL_StatusTypeDef ret;
	uint32_t command;
	uint8_t command_buffer[4] = {0, 0, 0, 0};

	// Set up a write operation
	command = KSZ9897_SPI_CMD_WRITE;

	// Set register address
	command |= (address << 5) & KSZ9897_SPI_CMD_ADDR;

	// Breaking command into byte array
	command_buffer[3] = (uint8_t)(command & 0xFF);
	command_buffer[2] = (uint8_t)((command >> 8) & 0xFF);
	command_buffer[1] = (uint8_t)((command >> 16) & 0xFF);
	command_buffer[0] = (uint8_t)((command >> 24) & 0xFF);

	//Pull the CS pin low
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

	// Writing 32-bit command
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&command_buffer, 4, 100);

	// Write 8-bit data
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&data, 1, 100);

	// Terminate the operation by raising the CS pin
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	// Return Error
	if(ret != HAL_OK)
	{
		return POE_ERR;
	}

	return POE_OK;
}

/**
 * @brief Read switch register (8 bits)
 * @param[in] handler SPI handler
 * @param[in] address Switch register address
 * @return Register value
 **/

uint8_t  ksz9897ReadSwitchReg8(SPI_HandleTypeDef *hspi, uint16_t address)
{
	HAL_StatusTypeDef ret;
	uint32_t command;
	uint8_t command_buffer[4] = {0, 0, 0, 0};
	uint8_t data_buffer[1];
	// Set up a read operation
	command = KSZ9897_SPI_CMD_READ;

	// Set register address
	command |= (address << 5) & KSZ9897_SPI_CMD_ADDR;

	// Breaking command into byte array
	command_buffer[3] = (uint8_t)(command & 0xFF);
	command_buffer[2] = (uint8_t)((command >> 8) & 0xFF);
	command_buffer[1] = (uint8_t)((command >> 16) & 0xFF);
	command_buffer[0] = (uint8_t)((command >> 24) & 0xFF);

	// Pull the CS pin low
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

	// Writing 32-bit command
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&command_buffer, 4, 100);

	// Reading 8-bit data
	ret = HAL_SPI_Receive(hspi, (uint8_t *)data_buffer, 1, 100);

	// Terminate the operation by raising the CS pin
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	// Return Error
	if(ret != HAL_OK)
	{
		return POE_RER;
	}

	// Return register value
	return data_buffer[0];
}

/**
 * @brief Write switch register (16 bits)
 * @param[in] handler SPI handler
 * @param[in] address Switch register address
 * @param[in] data Register value
 * @return  error code
 **/

poe_err ksz9897WriteSwitchReg16(SPI_HandleTypeDef *hspi, uint16_t address, uint16_t data)
{
	HAL_StatusTypeDef ret;
	uint32_t command;
	uint8_t command_buffer[4] = {0, 0, 0, 0};
	uint8_t data_buffer[2] = {0, 0};

	// Set up a write operation
	command = KSZ9897_SPI_CMD_WRITE;
	//Set register address
	command |= (address << 5) & KSZ9897_SPI_CMD_ADDR;

	// Breaking command into byte array
	command_buffer[3] = (uint8_t)(command & 0xFF);
	command_buffer[2] = (uint8_t)((command >> 8) & 0xFF);
	command_buffer[1] = (uint8_t)((command >> 16) & 0xFF);
	command_buffer[0] = (uint8_t)((command >> 24) & 0xFF);

	// Breaking data into byte array
	data_buffer[1] = (uint8_t)(command & 0xFF);
	data_buffer[0] = (uint8_t)((command >> 8) & 0xFF);

	// Pull the CS pin low
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

	// Write 32-bit command
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&command_buffer, 4, 100);

	// Write 16-bit data
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&data_buffer, 2, 100);

	//Terminate the operation by raising the CS pin
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	// Return Error
	if(ret != HAL_OK)
	{
		return POE_ERR;
	}

	return POE_OK;
}

/**
 * @brief Read switch register (16 bits)
 * @param[in] handler SPI handler
 * @param[in] address Switch register address
 * @return Register value
 **/

uint16_t ksz9897ReadSwitchReg16(SPI_HandleTypeDef *hspi, uint16_t address)
{
	HAL_StatusTypeDef ret;
	uint32_t command;
	uint8_t command_buffer[4] = {0, 0, 0, 0};
	uint8_t data_buffer[2];
	uint16_t data = 0;
	// Set up a read operation
	command = KSZ9897_SPI_CMD_READ;

	// Set register address
	command |= (address << 5) & KSZ9897_SPI_CMD_ADDR;

	// Breaking command into byte array
	command_buffer[3] = (uint8_t)(command & 0xFF);
	command_buffer[2] = (uint8_t)((command >> 8) & 0xFF);
	command_buffer[1] = (uint8_t)((command >> 16) & 0xFF);
	command_buffer[0] = (uint8_t)((command >> 24) & 0xFF);

	// Pull the CS pin low
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

	// Writing 32-bit command
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&command_buffer, 4, 100);

	// Reading 8-bit data
	ret = HAL_SPI_Receive(hspi, (uint8_t *)data_buffer, 2, 100);

	// Terminate the operation by raising the CS pin
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	// Return Error
	if(ret != HAL_OK)
	{
		return POE_RER;
	}

	// Merging received bytes
	data = data_buffer[0] << 8;
	data |= data_buffer[1];

	// Return register value
	return data;
}

/**
 * @brief Write switch register (32 bits)
 * @param[in] handler SPI handler
 * @param[in] address Switch register address
 * @param[in] data Register value
 * @return  error code
 **/

poe_err ksz9897WriteSwitchReg32(SPI_HandleTypeDef *hspi, uint16_t address, uint32_t data)
{
	HAL_StatusTypeDef ret;
	uint32_t command;
	uint8_t command_buffer[4] = {0, 0, 0, 0};
	uint8_t data_buffer[4] = {0, 0, 0, 0};

	//Set up a write operation
	command = KSZ9897_SPI_CMD_WRITE;
	//Set register address
	command |= (address << 5) & KSZ9897_SPI_CMD_ADDR;

	// Breaking command into byte array
	command_buffer[3] = (uint8_t)(command & 0xFF);
	command_buffer[2] = (uint8_t)((command >> 8) & 0xFF);
	command_buffer[1] = (uint8_t)((command >> 16) & 0xFF);
	command_buffer[0] = (uint8_t)((command >> 24) & 0xFF);

	// Breaking data into byte array
	data_buffer[3] = (uint8_t)(data & 0xFF);
	data_buffer[2] = (uint8_t)((data >> 8) & 0xFF);
	data_buffer[1] = (uint8_t)((data >> 16) & 0xFF);
	data_buffer[0] = (uint8_t)((data >> 24) & 0xFF);

	// Pull the CS pin low
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

	// Write 32-bit command
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&command_buffer, 4, 100);

	// Write 16-bit data
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&data_buffer, 4, 100);

	// Terminate the operation by raising the CS pin
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	// Return Error
	if(ret != HAL_OK)
	{
		return POE_ERR;
	}

	return POE_OK;
}

/**
 * @brief Read switch register (32 bits)
 * @param[in] handler SPI handler
 * @param[in] address Switch register address
 * @return Register value
 **/

uint32_t ksz9897ReadSwitchReg32(SPI_HandleTypeDef *hspi, uint16_t address)
{
	HAL_StatusTypeDef ret;
	uint32_t command;
	uint8_t command_buffer[4] = {0, 0, 0, 0};
	uint8_t data_buffer[4];
	uint32_t data = 0;
	// Set up a read operation
	command = KSZ9897_SPI_CMD_READ;

	// Set register address
	command |= (address << 5) & KSZ9897_SPI_CMD_ADDR;

	// Breaking command into byte array
	command_buffer[3] = (uint8_t)(command & 0xFF);
	command_buffer[2] = (uint8_t)((command >> 8) & 0xFF);
	command_buffer[1] = (uint8_t)((command >> 16) & 0xFF);
	command_buffer[0] = (uint8_t)((command >> 24) & 0xFF);

	// Pull the CS pin low
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

	// Writing 32-bit command
	ret = HAL_SPI_Transmit(hspi, (uint8_t *)&command_buffer, 4, 100);

	// Reading 8-bit data
	ret = HAL_SPI_Receive(hspi, (uint8_t *)data_buffer, 4, 100);

	// Terminate the operation by raising the CS pin
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

	// Return Error
	if(ret != HAL_OK)
	{
		return POE_RER;
	}

	// Merging received bytes
	data = data_buffer[0] << 24;
	data |= data_buffer[1] << 16;
	data |= data_buffer[2] << 8;
	data |= data_buffer[3];

	// Return register value
	return data;
}

/**
 * @brief Write PHY register
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @param[in] data Register value
 * @return  error code
 **/

poe_err ksz9897WritePhyReg(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t address, uint16_t data)
{
	poe_err ret;
	uint16_t n;

	// The SPI interface provides access to all PHY registers
	n = KSZ9897_PORTn_ETH_PHY_REG(port, address);
	// Write the 16-bit value
	ret = ksz9897WriteSwitchReg16(hspi, n, data);

	// Return Error
	return ret;
}

/**
 * @brief Read PHY register
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] address PHY register address
 * @return Register value
 **/

uint16_t ksz9897ReadPhyReg(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t address)
{
	uint16_t n;
	uint16_t data;

	// The SPI interface provides access to all PHY registers
	n = KSZ9897_PORTn_ETH_PHY_REG(port, address);
	// Read the 16-bit value
	data = ksz9897ReadSwitchReg16(hspi, n);

	// Return Error
	if(data == POE_RER)
	{
		return POE_RER;
	}

	// Return register value
	return data;
}

/**
 * @brief Write MMD register
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @param[in] data Register value
 * @return  error code
 **/

poe_err ksz9897WriteMmdReg(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t devAddr, uint16_t regAddr, uint16_t data)
{
	poe_err ret;

	// Select register operation
	ret = ksz9897WritePhyReg(hspi, port, KSZ9897_MMDACR, KSZ9897_MMDACR_FUNC_ADDR | (devAddr & KSZ9897_MMDACR_DEVAD));

	// Write MMD register address
	ret = ksz9897WritePhyReg(hspi, port, KSZ9897_MMDAADR, regAddr);

	// Select data operation
	ret = ksz9897WritePhyReg(hspi, port, KSZ9897_MMDACR, KSZ9897_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & KSZ9897_MMDACR_DEVAD));

	// Write the content of the MMD register
	ret = ksz9897WritePhyReg(hspi, port, KSZ9897_MMDAADR, data);

	// Return Error
	return ret;
}

/**
 * @brief Read MMD register
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] devAddr Device address
 * @param[in] regAddr Register address
 * @return Register value
 **/

uint16_t ksz9897ReadMmdReg(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t devAddr, uint16_t regAddr)
{
	// Select register operation
	ksz9897WritePhyReg(hspi, port, KSZ9897_MMDACR, KSZ9897_MMDACR_FUNC_ADDR | (devAddr & KSZ9897_MMDACR_DEVAD));

	// Write MMD register address
	ksz9897WritePhyReg(hspi, port, KSZ9897_MMDAADR, regAddr);

	// Select data operation
	ksz9897WritePhyReg(hspi, port, KSZ9897_MMDACR, KSZ9897_MMDACR_FUNC_DATA_NO_POST_INC | (devAddr & KSZ9897_MMDACR_DEVAD));

	// Read the content of the MMD register
	return ksz9897ReadPhyReg(hspi, port, KSZ9897_MMDAADR);
}

/**
 * @brief Set port state
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] state Port state
 **/

poe_err ksz9897SetPortState(SPI_HandleTypeDef *hspi, uint8_t port, SwitchPortState state)
{
	poe_err ret;
	uint8_t temp;

	// Check port number
	if(port >= KSZ9897_PORT1 && port <= KSZ9897_PORT5)
	{
		// Read MSTP state register
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_MSTP_STATE(port));

		// Update port state
		switch(state)
		{
		// Listening state
		case SWITCH_PORT_STATE_LISTENING:
			temp &= ~KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN;
			temp |= KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN;
			temp |= KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS;
			break;

			// Learning state
		case SWITCH_PORT_STATE_LEARNING:
			temp &= ~KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN;
			temp &= ~KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN;
			temp &= ~KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS;
			break;

			// Forwarding state
		case SWITCH_PORT_STATE_FORWARDING:
			temp |= KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN;
			temp |= KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN;
			temp &= ~KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS;
			break;

			// Disabled state
		default:
			temp &= ~KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN;
			temp &= ~KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN;
			temp |= KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS;
			break;
		}
		// Write the value back to MSTP state register
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_MSTP_STATE(port), temp);

		// Return Error
		return ret;
	}

	return POE_WR;
}

/**
 * @brief Get port state
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @return Port state
 **/

SwitchPortState ksz9897GetPortState(SPI_HandleTypeDef *hspi, uint8_t port)
{
	uint8_t temp;
	SwitchPortState state;

	//Check port number
	if(port >= KSZ9897_PORT1 && port <= KSZ9897_PORT5)
	{
		// Read MSTP state register
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_MSTP_STATE(port));

		// Check port state
		if((temp & KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN) == 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN) == 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS) != 0)
		{
			// Disabled state
			state = SWITCH_PORT_STATE_DISABLED;
		}
		else if((temp & KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN) == 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN) != 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS) != 0)
		{
			// Listening state
			state = SWITCH_PORT_STATE_LISTENING;
		}
		else if((temp & KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN) == 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN) == 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS) == 0)
		{
			// Learning state
			state = SWITCH_PORT_STATE_LEARNING;
		}
		else if((temp & KSZ9897_PORTn_MSTP_STATE_TRANSMIT_EN) != 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_RECEIVE_EN) != 0 &&
				(temp & KSZ9897_PORTn_MSTP_STATE_LEARNING_DIS) == 0)
		{
			// Forwarding state
			state = SWITCH_PORT_STATE_FORWARDING;
		}
		else
		{
			// Unknown state
			state = SWITCH_PORT_STATE_UNKNOWN;
		}
	}
	else
	{
		// The specified port number is not valid
		state = SWITCH_PORT_STATE_DISABLED;
	}

	// Return port state
	return state;
}

/**
 * @brief Get link state
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @return Link state
 **/

bool ksz9897GetLinkState(SPI_HandleTypeDef *hspi, uint8_t port)
{
	uint16_t value;
	bool linkState;

	// Check port number
	if(port >= KSZ9897_PORT1 && port <= KSZ9897_PORT5)
	{
		// Any link failure condition is latched in the BMSR register. Reading
		// the register twice will always return the actual link status
		value = ksz9897ReadPhyReg(hspi, port, KSZ9897_BMSR);
		value = ksz9897ReadPhyReg(hspi, port, KSZ9897_BMSR);

		// Retrieve current link state
		linkState = (value & KSZ9897_BMSR_LINK_STATUS) ? true : false;
	}
	else
	{
		// The specified port number is not valid
		linkState = false;
	}

	// Return link status
	return linkState;
}

/**
 * @brief Get link speed
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @return Link speed
 **/

uint32_t ksz9897GetLinkSpeed(SPI_HandleTypeDef *hspi, uint8_t port)
{
	uint8_t type;
	uint16_t value;
	uint32_t linkSpeed;

	// Check port number
	if(port >= KSZ9897_PORT1 && port <= KSZ9897_PORT5)
	{
		// Read PHY control register
		value = ksz9897ReadPhyReg(hspi, port, KSZ9897_PHYCON);

		// Retrieve current link speed
		if((value & KSZ9897_PHYCON_SPEED_1000BT) != 0)
		{
			// 1000BASE-T

		}
		else if((value & KSZ9897_PHYCON_SPEED_100BTX) != 0)
		{
			// 100BASE-TX
			linkSpeed = KSZ_LINK_SPEED_100MBPS;
		}
		else if((value & KSZ9897_PHYCON_SPEED_10BT) != 0)
		{
			// 10BASE-T
			linkSpeed = KSZ_LINK_SPEED_10MBPS;
		}
		else
		{
			// The link speed is not valid
			linkSpeed = KSZ_LINK_SPEED_UNKNOWN;
		}
	}
	else if(port == KSZ9897_PORT6)
	{
		// Read port 6 XMII control 1 register
		value = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL1);

		// Retrieve host interface type
		type = value & KSZ9897_PORTn_XMII_CTRL1_IF_TYPE;

		// Gigabit interface?
		if(type == KSZ9897_PORTn_XMII_CTRL1_IF_TYPE_RGMII &&
				(value & KSZ9897_PORTn_XMII_CTRL1_SPEED_1000) == 0)
		{
			// 1000 Mb/s mode
			linkSpeed = KSZ_LINK_SPEED_1GBPS;
		}
		else
		{
			// Read port 6 XMII control 0 register
			value = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORT6_XMII_CTRL0);

			// Retrieve host interface speed
			if((value & KSZ9897_PORTn_XMII_CTRL0_SPEED_10_100) != 0)
			{
				// 100 Mb/s mode
				linkSpeed = KSZ_LINK_SPEED_100MBPS;
			}
			else
			{
				//10 Mb/s mode
				linkSpeed = KSZ_LINK_SPEED_10MBPS;
			}
		}

	}
	else
	{
		// The specified port number is not valid
		linkSpeed = KSZ_LINK_SPEED_UNKNOWN;
	}

	//Return link status
	return linkSpeed;
}

/**
 * @brief Start and set the authentication mode for ACL
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] enable service on/off
 * @param[in] mode authentication mode
 * @return  error code
 **/

poe_err ksz9897EnableACL(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t enable, uint8_t mode)
{
	poe_err ret;
	uint8_t temp;

	if(port >= 1 && port <= 6)
	{
		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_AUTH_CTRL(port));

		if(enable) {
			temp |= 0x04;
		}
		else {
			temp &= 0xFB;
		}

		temp &= 0xFC;

		if(mode == 2) {
			temp |= 0x01;
		}

		ret = ksz9897WriteSwitchReg8(HSPI, KSZ9897_PORTn_AUTH_CTRL(port), temp);

		// Return Error
		return ret;
	}

	return POE_WR;
}

/**
 * @brief Writes ACL Table Entry Of Port
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] index entry index
 * @param[in] entry ACL Table entry structure
 * @return  error code
 **/

poe_err ksz9897WriteACLTableEntry(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t index, ACLTableEntry *entry)
{
	poe_err ret;
	uint8_t temp;

	if((port >= 1 && port <= 6) && index <= 15)
	{
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS0(port), entry->FRN);
		temp = 0x00;
		switch(entry->MD) {
			case LAYER2_MAC_ETHERTYPE_MATCHING:
				temp |= 0x10;
				break;
			case LAYER3_IP_MATCHING:
				temp |= 0x20;
				break;
			case LAYER4_TCP_UDP_IP_MATCHING:
				temp |= 0x30;
				break;
			default:
				temp &= 0xCF;
				break;
		}
		switch(entry->ENB) {
			case 1:
				temp |= 0x04;
				break;
			case 2:
				temp |= 0x08;
				break;
			case 3:
				temp |= 0x0C;
				break;
			default:
				temp &= 0xF3;
				break;
		}

		if(entry->SD == SOURCE_ADRESS) {
			temp |= 0x02;
		}
		if(entry->EQ == PERMIT) {
			temp |= 0x01;
		}

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS1(port), temp);

		if(entry->MD == LAYER2_MAC_ETHERTYPE_MATCHING)
		{
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port), entry->MAC_Address[0]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port), entry->MAC_Address[1]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port), entry->MAC_Address[2]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port), entry->MAC_Address[3]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS6(port), entry->MAC_Address[4]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS7(port), entry->MAC_Address[5]);

			if(entry->ENB != COMPARE_MAC_ONLY)
			{
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS8(port), entry->EtherType[0]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS9(port), entry->EtherType[1]);
			}
		}
		else if(entry->MD == LAYER3_IP_MATCHING)
		{
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port), entry->IP_Address[0]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port), entry->IP_Address[1]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port), entry->IP_Address[2]);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port), entry->IP_Address[3]);
			if(entry->ENB == COMPARE_IPV4_SOURCE_WITH_MASK)
			{
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS6(port), entry->IP_Mask[0]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS7(port), entry->IP_Mask[1]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS8(port), entry->IP_Mask[2]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS9(port), entry->IP_Mask[3]);
			}
		}

		else if(entry->MD == LAYER4_TCP_UDP_IP_MATCHING)
		{
			if(entry->ENB == COMPARE_TCP_SD_PORT || entry->ENB == COMPARE_UDP_SD_PORT)
			{
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port), entry->MaxPort[0]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port), entry->MaxPort[1]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port), entry->MinPort[0]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port), entry->MinPort[1]);
			}
			else if(entry->ENB == COMPARE_TCP_SEQUENCE_NUMBER)
			{
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port), entry->TCPSnumber[0]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port), entry->TCPSnumber[1]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port), entry->TCPSnumber[2]);
				ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port), entry->TCPSnumber[3]);
			}

			temp = (0x00 & 0xF3) | ((entry->PC << 2) & 0x0C);
			temp = (entry->PRO & 0xFE) | ((temp >> 7) & 0x01);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS6(port), temp);

			temp = ((temp & 0x00) | entry->PRO << 1);
			if(entry->FME)
			{
				temp |= 0x01;
			}
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS7(port), temp);

			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS8(port), entry->FMSK);
			ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS9(port), entry->FLAG);
		}

		temp = (0x00 & 0x3F) | ((entry->Action_Rule_PM << 6) & 0xC0);
		temp |= ((entry->Action_Rule_P & 0x07) << 3);

		if(entry->Action_Rule_RPE)
		{
			temp |= 0x4;
		}
		temp |= ((entry->Action_Field_RP & 0x06) >> 1);

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS10(port), temp);

		temp = (0x00 & 0x7F) | ((entry->Action_Field_RP << 7) & 0x80);
		temp |= ((entry->Action_Field_MM << 5) & 0x60);

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS11(port), temp);

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS13(port), entry->Action_Field_Forward);

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS15(port), entry->Rule_Set[0]);
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS14(port), entry->Rule_Set[1]);

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_BYTE_EN_MSB(port), 0xFF);
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_BYTE_EN_LSB(port), 0xF7);


		temp = 0x00;
		temp |= index;
		temp |= 0x10;
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS_CTRL0(port), temp);

		temp = 0x00;
		while((temp & (1 << 5)) != 0) {

			temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS_CTRL0(port));
			HAL_Delay(1);
		}

		// Return Error
		return ret;

	}

	return POE_WR;
}

/**
 * @brief Read ACL Table Entry Of Port
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] index entry index
 * @return  error code
 **/

poe_err ksz9897ReadACLTableEntry(SPI_HandleTypeDef *hspi, uint8_t port, uint8_t index, ACLTableEntry *entry)
{
	poe_err ret;
	uint8_t temp;

	if((port >= 1 && port <= 6) && index <= 15)
	{

		ksz9897EnableACL(hspi, port, 0, poeSettings.portACL[port - 1][1]);

		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_BYTE_EN_MSB(port), 0xFF);
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_BYTE_EN_LSB(port), 0xF7);

		temp = 0x00;
		temp |= index;
		ret = ksz9897WriteSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS_CTRL0(port), temp);

		temp = 0x00;
		while((temp & (1 << 4)) != 0) {

			temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS_CTRL0(port));
			HAL_Delay(1);
		}


		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS0(port));
		entry->FRN = temp;

		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS1(port));
		entry->MD = (temp >> 4) & 0b11;
		entry->ENB = (temp >> 2) & 0b11;
		entry->SD = (temp >> 1) & 0b1;
		entry->EQ = temp & 0b1;

		switch(entry->MD) {
			case LAYER2_MAC_ETHERTYPE_MATCHING:
				entry->MAC_Address[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port));
			    entry->MAC_Address[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port));
				entry->MAC_Address[2] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port));
				entry->MAC_Address[3] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port));
				entry->MAC_Address[4] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS6(port));
				entry->MAC_Address[5] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS7(port));
				if(entry->ENB != COMPARE_MAC_ONLY)
				{
					entry->EtherType[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS8(port));
					entry->EtherType[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS9(port));
				}
				break;
			case LAYER3_IP_MATCHING:
				entry->IP_Address[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port));
				entry->IP_Address[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port));
				entry->IP_Address[2] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port));
				entry->IP_Address[3] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port));
				if(entry->ENB == COMPARE_IPV4_SOURCE_WITH_MASK)
				{
					entry->IP_Mask[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS6(port));
					entry->IP_Mask[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS7(port));
					entry->IP_Mask[2] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS8(port));
					entry->IP_Mask[3] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS9(port));
				}
				break;
			case LAYER4_TCP_UDP_IP_MATCHING:
				if(entry->ENB == COMPARE_TCP_SD_PORT || entry->ENB == COMPARE_UDP_SD_PORT)
				{
					entry->MaxPort[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port));
					entry->MaxPort[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port));
					entry->MinPort[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port));
					entry->MinPort[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port));
				}
				else if(entry->ENB == COMPARE_TCP_SEQUENCE_NUMBER)
				{
					entry->TCPSnumber[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS2(port));
					entry->TCPSnumber[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS3(port));
					entry->TCPSnumber[2] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS4(port));
					entry->TCPSnumber[3] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS5(port));
				}

				temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS6(port));
				entry->PC = (temp >> 1 ) & 0x03;
				entry->PRO = (temp << 7 ) & 0x80;
				temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS7(port));
				entry->PRO |= (temp >> 1 ) & 0x7F;
				entry->FME = temp & 0x01;

				entry->FMSK = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS8(port));
				entry->FLAG = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS9(port));

				break;
			default:
				break;
		}

		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS10(port));
		entry->Action_Rule_PM = (temp >> 6 ) & 0x03;
		entry->Action_Rule_P = (temp >> 3 ) & 0x07;
		entry->Action_Rule_RPE =  (temp >> 2 ) & 0x01;
		entry->Action_Field_RP = (temp & 0x03) << 1;

		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS11(port));
		entry->Action_Field_RP |= (temp >> 7) & 0x01;
		entry->Action_Field_MM = (temp >> 5) & 0x03;

		entry->Action_Field_Forward = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS13(port));
		entry->Rule_Set[0] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS15(port));
		entry->Rule_Set[1] = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_ACL_ACCESS14(port));

		ret = ksz9897EnableACL(hspi, port, poeSettings.portACL[port - 1][0] , poeSettings.portACL[port - 1][1]);

		// Return Error
		return ret;
	}

	return POE_WR;
}

/**
 * @brief Read ACL Tables from Flash & Write to KSZ9897
 * @param[in] handler SPI handler
 * @return  error code
 **/

poe_err ksz9897SetACLTables(SPI_HandleTypeDef *hspi)
{
	poe_err ret = POE_OK;
	KSZ9897AdvanceFeatures sheet;

	memset(&sheet, '\0', sizeof(sheet));
	readACLSheetToFlash(&sheet, sizeof(KSZ9897AdvanceFeatures));

	for(int port = 1; port <= 6 ; port++)
	{
		if(poeSettings.portACL[port - 1][0] == 1)
		{
			for(int entry = 0; entry <=15; entry++)
			{
				if(sheet.aclSheet[port - 1][entry].MD > 0 && sheet.aclSheet[port - 1][entry].MD < 4)
				{
					ret = ksz9897WriteACLTableEntry(hspi, port, entry, &sheet.aclSheet[port - 1][entry]);
				}
			}

			ret = ksz9897EnableACL(hspi, port, poeSettings.portACL[port - 1][0] , poeSettings.portACL[port - 1][1]);
		}
	}

	// Return Error
	return ret;
}

/**
 * @brief Read VLAN Groups from flash & Set them
 * @param[in] handler SPI handler
 * @return  error code
 **/

poe_err ksz9897SetVLAN(SPI_HandleTypeDef *hspi)
{
	KSZ9897AdvanceFeatures group;
	poe_err ret = POE_OK;

	memset(&group, '\0', sizeof(group));
	readACLSheetToFlash(&group, sizeof(KSZ9897AdvanceFeatures));

	for(int port = 1; port <= 6 ; port++)
	{
		if(group.vlanGroups[port - 1].vlanService == 1)
		{
			 ret = ksz9897WriteVLANGroup(hspi, port, &group.vlanGroups[port - 1]);
		}
	}

	// Return Error
	return ret;
}

/**
 * @brief Write VLAN Group
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @param[in] group VLAN group structure
 * @return  error code
 **/

poe_err ksz9897WriteVLANGroup(SPI_HandleTypeDef *hspi, uint8_t port, VLANGroup *group)
{
	if(port >= 1 && port <= 6)
	{
		uint8_t temp;
		uint8_t mask[6] = {0x7E,0x7D,0x7B,0x77,0x6F,0x5F};
		poe_err ret = POE_OK;

		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_CTRL1(port));

		if(temp == POE_RER) {
			ret = POE_RER;
		}
		else {
			ret = POE_OK;
		}

		for(int i = 0; i <= 5 ; i++)
		{
			temp = (group->vlanPortGroup[i] << i)  | (temp & mask[i]);
		}

		// Return Error
		return ret;
	}

	return POE_WR;
}

/**
 * @brief Read VLAN Group
 * @param[in] handler SPI handler
 * @param[in] port Port number
 * @return  error code
 **/

poe_err ksz9897ReadVLANGroup(SPI_HandleTypeDef *hspi, uint8_t port, VLANGroup *group)
{
	if(port >= 1 && port <= 6)
	{
		uint8_t temp;
		poe_err ret = POE_OK;

		temp = ksz9897ReadSwitchReg8(hspi, KSZ9897_PORTn_CTRL1(port));

		if(temp == POE_RER) {
			ret = POE_RER;
		}
		else {
			ret = POE_OK;
		}

		for(int i = 0; i <= 5 ; i++)
		{
			group->vlanPortGroup[i] = (temp >> i) & 0x01;
		}

		// Return Error
		return ret;
	}

	return POE_WR;
}
