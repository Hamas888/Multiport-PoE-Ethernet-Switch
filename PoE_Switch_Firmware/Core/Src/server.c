 /*
 ====================================================================================================
 * File:		http_ssi.c
 * Author:		AMS-IOT
 * Version:	    Rev_1.0.0
 * Date:		Dec 26, 2023
 * Brief:		This file contains the HTTP Server side for POE switch
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
#include "ksz9897.h"
#include "server.h"

/* Defining the tags that will be updated */
char const* TAGCHAR[]={ "l11", "l12", "l13", "l14",          						// port 1: speed, state, link, PD class
					    "l21", "l22", "l23", "l24",          						// port 2: speed, state, link, PD class
					    "l31", "l32", "l33", "l34",          						// port 3: speed, state, link, PD class
					    "l41", "l42", "l43", "l44",          						// port 4: speed, state, link, PD class
					    "l51", "l52", "l53", "l54" };        						// port 5: speed, state, link, PD class

char const** TAGS=TAGCHAR;                                   						// Tags String

/* Defining the handlers for CGI */
const tCGI LOGIN_CGI = {"/login.cgi", CGILogin_Handler};							// CGI Handler for login request
const tCGI SETTING_CGI = {"/setting.cgi", CGISetting_Handler};						// CGI Handler for settings changes
const tCGI UPDATE_PAGE_CGI = {"/update_page.cgi", CGIUpdatePage_Handler};			// CGI Handler for status updates
const tCGI UPDATE_REQUEST_CGI = {"/updateRequest.cgi", CGIUpdateRequest_Handler};	// CGI Handler for firmware update request
const tCGI RESET_CGI = {"/reset.cgi", CGIReset_Handler};							// CGI Handler for device reset

tCGI CGI_TAB[5];                                                                    // Array For ALL CGI Handlers

/* To handle login and settings change */
char username[10];                                                                  // Holds Received User Name
char password[10];																	// Holds Received Password
char setting[5][40];																// Holds Received Settings
uint8_t portS[5];																	// Holds Ports Link Status

/* To handle OTA */
uint8_t updateState = 0;															// Used To Represent The State Of Firmware Update
uint8_t firmwareChunks = 0;															// Used To Hold The Total Number Of Firmware Chunks
uint8_t firmwareChunksRecived = 0;													// Used To Hold The Chunk Number Received Recently
uint32_t crc = 0;																	// Used To Hold The CRC Value Received With The Chunk

/* To handle ACL & VLAN Entries */
bool aclFlag = false;																// Indicates ACL Entries Are Received
bool vlanFlag = false;																// Indicates VLAN Group Is Received
uint8_t entries = 0;																// Used To Hold Total Number of ACL Entries Received



/**
 * @brief Replace characters from a string
 * @param[in] string
 * @param[in] string that needs to be replaced
 * @param[in] string to replace with
 **/

void replaceSubstring(char *str, const char *find, const char *replace)
{
	char *pos = str;
	while ((pos = strstr(pos, find)) != NULL)
	{
		memmove(pos + strlen(replace), pos + strlen(find), strlen(pos + strlen(find)) + 1);
		strncpy(pos, replace, strlen(replace));
		pos += strlen(replace);
	}
}

/**
 * @brief Apply and update the received settings
 * @return  error code
 **/

poe_err parseSwitchSetting()
{
	poe_err ret;
	uint8_t count;

	char status[5][11];		// To Store Status Of Each Port
	char speed[5][11];		// To Store Speed Of Each Port
	char pconfig[5][11];	// To Store Power Configuration Of Each Port
	char penable[5][11];	// To Store Power On/Off Of Each Port

	// Clearing Buffers
	memset(status, '\0', sizeof(status));
	memset(speed, '\0', sizeof(speed));
	memset(pconfig, '\0', sizeof(pconfig));
	memset(penable, '\0', sizeof(penable));

	// Loop To Separate Ports Settings
	for(int i = 0; i < 5; i++)
	{
		char *param = strtok(setting[i], ",");
		count = 0;
		// Loop To Separate Ports Parameter
		while (count <4 )
		{
			if(count ==0)
			{
				strcpy(speed[i], param);
				replaceSubstring(speed[i], "%20", " ");
			}
			else if(count == 1)
			{
				strcpy(status[i], param);
			}
			else if(count == 2)
			{
				strcpy(pconfig[i], param);
			}
			else if(count == 3)
			{
				strcpy(penable[i], param);
			}
			count++;
			// Get the next parameter
			param = strtok(NULL, ",");
		}
	}

	// Setting the parameters
	ret = setPortState(status);
	ret = setPortSpeed(speed);
	ret = setPortPower(pconfig,penable);

	// Returning Error Code
	return ret;
}

/**
 * @brief Set the ports state
 * @param[in] changed states
 * @return  error code
 **/

poe_err setPortState(char param[][11])
{
	poe_err ret;

	// Loop To Set Each Port State
	for(int i = 0; i < 5; i++)
	{
		if (strcmp(param[i], "Forwarding") == 0)
		{
			ret = ksz9897SetPortState(HSPI, i+1, SWITCH_PORT_STATE_FORWARDING);
		}
		else if (strcmp(param[i], "Learning") == 0)
		{
			ret = ksz9897SetPortState(HSPI, i+1, SWITCH_PORT_STATE_LEARNING);
		}
		else if (strcmp(param[i], "Listening") == 0)
		{
			ret = ksz9897SetPortState(HSPI, i+1, SWITCH_PORT_STATE_LISTENING);
		}
		else if (strcmp(param[i], "Blocking") == 0)
		{
			ret = ksz9897SetPortState(HSPI, i+1, SWITCH_PORT_STATE_BLOCKING);
		}
		else if (strcmp(param[i], "Disable") == 0)
		{
			ret = ksz9897SetPortState(HSPI, i+1, SWITCH_PORT_STATE_DISABLED);
		}
		strcpy(poeSettings.status[i], param[i]);
	}

	// Returning Error Code
	return ret;
}

/**
 * @brief Set the port speed
 * @param[in] changed speed
 * @return  error code
 **/

poe_err setPortSpeed(char param[][11])
{
	poe_err ret;

	// Loop To Set Each Port Speed
	for(int i = 0; i < 5; i++)
	{
		if (strcmp(param[i], "10 MBps") == 0)
		{
			ret = Ksz9897Portspeed10(HSPI, i+1);
		}
		else if (strcmp(param[i], "100 MBps") == 0)
		{
			ret = Ksz9897Portspeed100(HSPI, i+1);
		}
		strcpy(poeSettings.speed[i], param[i]);
	}

	// Returning Error Code
	return ret;
}

/**
 * @brief Set the port PD settings
 * @param[in] changed power class
 * @param[in] changed power enabled or not
 * @return  error code
 **/

poe_err setPortPower(char param1[][11], char param2[][11])
{
	poe_err ret;
	bool classOk = false;
	TPS23881 PSE;

	PSE.i2chandle = HI2C;

	// Loop To Set Power For Each Port
	for(int i = 0; i < 5; i++)
	{
		// Checks IF To Set Port For Max 30W
		if (strcmp(param1[i], "ieee3at") == 0)
		{
			// Checks If The Connected Device To Port Under 30W
			if((tpsPortsClassificationBuffer[i]) <= CLASS_0 && (tpsPortsClassificationBuffer[i] > CLASS_UNKNOWN ))
			{
				classOk = true;
			}
		}
		// Checks IF To Set Port For Max 90W
		else if (strcmp(param1[i], "ieee3bt") == 0)
		{
			// Checks If The Connected Device To Port Surplus 30W
			if((tpsPortsClassificationBuffer[i] > CLASS_0) && (tpsPortsClassificationBuffer[i] < CLASS_MISMATCH))
			{
				classOk = true;
			}
		}

		// Checks If Safety Checks OK Then Disable Or Enable The Power
		if(classOk)
		{
			if (strcmp(param2[i], "enable") == 0)
			{
				ret = TPS23881_PowerEnable(&PSE, i + 1, ENABLE, DEVICE_ADDR_A);
			}
			else if (strcmp(param2[i], "disable") == 0)
			{
				ret = TPS23881_PowerEnable(&PSE, i + 1, Disable, DEVICE_ADDR_A);
			}

			classOk = false;
		}
	}

	// Returning Error Code
	if(ret != POE_OK)
	{
		return ret;
	}

	return POE_OK;
}

/**
 * @brief Fetch port Status
 * @param[in] port number
 * @return port state
 **/

char* portState(int port)
{
	SwitchPortState state;

	// Fetch The State
	state = ksz9897GetPortState(HSPI, port);

	// Return Appropriate String According To State
	switch(state)
	{
		case SWITCH_PORT_STATE_DISABLED:
			return "Disable";
			break;
		case SWITCH_PORT_STATE_LISTENING:
			return "Listening";
			break;
		case SWITCH_PORT_STATE_LEARNING:
			return "Learning";
			break;
		case SWITCH_PORT_STATE_FORWARDING:
			return "Forwarding";
			break;
		default:
			return "Unknown";
			break;
	}
}

/**
 * @brief Fetch port speed
 * @param[in] port number
 * @return port speed
 **/

char* portSpeed(int port)
{
	KszLinkSpeed speed;

	// Fetch The Speed
	speed = ksz9897GetLinkSpeed(HSPI, port);

	// Return Appropriate String According To Speed
	switch(speed)
	{
	case KSZ_LINK_SPEED_10MBPS:
		return "10 MBps";
		break;
	case KSZ_LINK_SPEED_100MBPS:
		return "100 MBps";
		break;
	default:
		return "Error";
		break;
	}
}

/**
 * @brief Fetch port class
 * @param[in] port number
 * @return port class
 **/

char* portClass(int port)
{
	// Return Class String According to Classification Buffer
	switch(tpsPortsClassificationBuffer[port - 1])
	  {
		  case CLASS_1:
			  return "Class 1";
			  break;
		  case CLASS_2:
			  return "Class 2";
			  break;
		  case CLASS_3:
			  return "Class 3";
			  break;
		  case CLASS_4:
			  return "Class 4";
			  break;
		  case CLASS_0 :
			  return "Class 0";
			  break;
		  case CLASS_5_4P_SINGLE:
			  return "Class 5";
			  break;
		  case CLASS_6_4P_SINGLE:
			  return "Class 6";
			  break;
		  case CLASS_7_4P_SINGLE:
			  return "Class 7";
			  break;
		  case CLASS_8_4P_SINGLE:
			  return "Class 8";
			  break;
		  case CLASS_4PLUS_TYPE1 :
			  return "Class 4P";
			  break;
		  case CLASS5_4P_DUAL :
			  return "Class 5D";
			  break;
		  default:
			  return "No";
			  break;
	  }
}

/**
 * @brief Update the tags on status page
 * @param[in] tag index
 * @param[in] pointer to tag value
 * @param[in] length of the tag
 * @return updated tag
 * Note: More Information in httpd.h
 **/

uint16_t ssi_handler (int iIndex, char *pcInsert, int iInsertLen)
{
	// Update The Tag
	switch (iIndex)
	{
		case 0:
			sprintf(pcInsert, "%s", portSpeed(1));
			return strlen(pcInsert);
			break;
		case 1:
			sprintf(pcInsert, "%s", portState(1));
			return strlen(pcInsert);
			break;
		case 2:
			sprintf(pcInsert, "%s", (ksz9897GetLinkState(HSPI, 1) ? "up" : "down"));
			return strlen(pcInsert);
			break;
		case 3:
			sprintf(pcInsert, "%s", portClass(1));
			return strlen(pcInsert);
			break;
		case 4:
			sprintf(pcInsert, "%s", portSpeed(2));
			return strlen(pcInsert);
			break;
		case 5:
			sprintf(pcInsert, "%s", portState(2));
			return strlen(pcInsert);
			break;
		case 6:
			sprintf(pcInsert, "%s", (ksz9897GetLinkState(HSPI, 2) ? "up" : "down"));
			return strlen(pcInsert);
			break;
		case 7:
			sprintf(pcInsert, "%s", portClass(2));
			return strlen(pcInsert);
			break;
		case 8:
			sprintf(pcInsert, "%s", portSpeed(3));
			return strlen(pcInsert);
			break;
		case 9:
			sprintf(pcInsert, "%s", portState(3));
			return strlen(pcInsert);
			break;
		case 10:
			sprintf(pcInsert, "%s", (ksz9897GetLinkState(HSPI, 3) ? "up" : "down"));
			return strlen(pcInsert);
			break;
		case 11:
			sprintf(pcInsert, "%s", portClass(3));
			return strlen(pcInsert);
			break;
		case 12:
			sprintf(pcInsert, "%s", portSpeed(4));
			return strlen(pcInsert);
			break;
		case 13:
			sprintf(pcInsert, "%s", portState(4));
			return strlen(pcInsert);
			break;
		case 14:
			sprintf(pcInsert, "%s", (ksz9897GetLinkState(HSPI, 4) ? "up" : "down"));
			return strlen(pcInsert);
			break;
		case 15:
			sprintf(pcInsert, "%s", portClass(4));
			return strlen(pcInsert);
			break;
		case 16:
			sprintf(pcInsert, "%s", portSpeed(5));
			return strlen(pcInsert);
			break;
		case 17:
			sprintf(pcInsert, "%s", portState(5));
			return strlen(pcInsert);
			break;
		case 18:
			sprintf(pcInsert, "%s", (ksz9897GetLinkState(HSPI, 5) ? "up" : "down"));
			return strlen(pcInsert);
			break;
		case 19:
			sprintf(pcInsert, "%s", portClass(5));
			return strlen(pcInsert);
			break;
		default :
			return 0;
			break;
	}

}

/**
 * @brief Handles login CGI request
 * @param[in] CGI index
 * @param[in] number of parameters
 * @param[in] parameters
 * @param[in] parameters values
 * @return response to the request
 * Note: More Information in httpd.h
 **/

const char *CGILogin_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	if (iIndex == 0)
	{
		for (int i=0; i<iNumParams; i++)
		{
			if (strcmp(pcParam[i], "username") == 0)  						// if the user name string is found
			{
				memset(username, '\0', 10);
				strcpy(username, pcValue[i]);
			}

			else if (strcmp(pcParam[i], "password") == 0)  					// if the password string is found
			{
				memset(password, '\0', 10);
				strcpy(password, pcValue[i]);
			}
		}
	}

	// Checks If User Name Is Correct
	if(strcmp(username, poeSettings.username) == 0)
	{
		// Checks If Password Is Correct
		if(strcmp(password, poeSettings.password) == 0)
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "---------Login Request Successful\r\n") );
#endif
			// Returns The Status Page (Success)
			return "/switch_status.shtml";
		}
	}
#ifdef SERIAL_DEBUG
	SerialDebug( strcpy((char*) debugBuff, "---------Login Request Failed\r\n") );
#endif
	// Returns The Home Page (Fail)
	return "/index.html";
}

/**
 * @brief Handles settings CGI request
 * @param[in] CGI index
 * @param[in] number of parameters
 * @param[in] parameters
 * @param[in] parameters values
 * @return response to the request
 * Note: More Information in httpd.h
 **/

const char *CGISetting_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	uint8_t ret = POE_OK;

	if (iIndex == 1)
	{
		memset(setting, '\0', sizeof(setting));

		for (int i=0; i<iNumParams; i++)
		{
			if (strcmp(pcParam[i], "port1") == 0)  							// if the port1 string is found
			{
				strcpy(setting[i], pcValue[i]);
			}
			else if (strcmp(pcParam[i], "port2") == 0)  					// if the port2 string is found
			{
				strcpy(setting[i], pcValue[i]);
			}
			else if (strcmp(pcParam[i], "port3") == 0)  					// if the port3 string is found
			{
				strcpy(setting[i], pcValue[i]);
			}
			else if (strcmp(pcParam[i], "port4") == 0)  					// if the port4 string is found
			{
				strcpy(setting[i], pcValue[i]);
			}
			else if (strcmp(pcParam[i], "port5") == 0)  					// if the port5 string is found
			{
				strcpy(setting[i], pcValue[i]);
			}
		}

		// updating settings and changing them
		ret = parseSwitchSetting();

		// Handle the Error
		if(ret)
		{
			// Returns The 404 Page (Fail)
			return "/404.html";
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "\n---------Error Occurred While Parsing Settings\r\n"));
#endif
		}

		// rewriting the updated settings to the flash
		writeSettingsToFlash(&poeSettings);
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Settings Updated & Saved\r\n"));
#endif
	}

	// Returns The Status Page (Success)
	return "/switch_status.shtml";
}

/**
 * @brief Handles status update CGI request
 * @param[in] CGI index
 * @param[in] number of parameters
 * @param[in] parameters
 * @param[in] parameters values
 * @return response to the request
 * Note: More Information in httpd.h
 **/

const char *CGIUpdatePage_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	bool change = false;
	uint8_t* tempState;

	if (iIndex == 2)
	{
		// Fetch The Ports Link Status
		tempState = ksz9897InterruptHandler(HSPI);

		// Loop To Determine Any Change
		for(int i = 0; i < 5; i++)
		{
			if(portS[i] != tempState[i])
			{
				change = true;
				poe_err ret;

				if(tempState[i]) {
					ret = ksz9897SetPortState(HSPI, i + 1, SWITCH_PORT_STATE_FORWARDING);
				}
				else {
					ret = ksz9897SetPortState(HSPI, i + 1, SWITCH_PORT_STATE_DISABLED);
				}

				// Handle Error
				if(ret != POE_OK)
				{

				}
			}
			portS[i] = tempState[i];
		}

	}

	// If Something Is Changed
	if(change)
	{
#ifdef SERIAL_DEBUG
		SerialDebug( strcpy((char*) debugBuff, "---------Change Detected Updating Status Page\r\n"));
#endif
		// Returns The Status Page (Refresh)
		return "/switch_status.shtml";
	}
	else
	{
		// Returns The Home Page (No Refresh)
		return "/index.html";
	}
}

/**
 * @brief Handles Firmware Update Check CGI request
 * @param[in] CGI index
 * @param[in] number of parameters
 * @param[in] parameters
 * @param[in] parameters values
 * @return response to the request
 * Note: More Information in httpd.h
 **/

const char *CGIUpdateRequest_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	if (iIndex == 3)
	{
		for (int i=0; i<iNumParams; i++)
		{
			if (strcmp(pcParam[i], "latestVersion") == 0)
			{
				// Compare The Device Version With Version Fetched
				uint8_t result = compareVersions(poeSettings.fV, pcValue[i]);

			    if (result == 2)
			    {
			    	// Returns The Update Page (Prompt User To Download New Firmware)
			    	return "/update.html";
			    }
			    else if (result < 2)
			    {
			    	// Returns The 404 Page (Prompt User That Device On latest Firmware)
			    	return "/404.html";
			    }
			}
		}
	}

	return "/404.html";
}

/**
 * @brief Handles Reset CGI request
 * @param[in] CGI index
 * @param[in] number of parameters
 * @param[in] parameters
 * @param[in] parameters values
 * @return response to the request
 * Note: More Information in httpd.h
 **/

const char *CGIReset_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	if (iIndex == 4)
	{
		for (int i=0; i<iNumParams; i++)
		{
			if (strcmp(pcParam[i], "update") == 0)
			{
				if (strcmp(pcValue[i], "1") == 0)
				{
					// Set The System Reset Flag
					systemResetFlag = true;
#ifdef SERIAL_DEBUG
					SerialDebug( strcpy((char*) debugBuff, "---------Rebooting\r\n") );
					SerialDebug( strcpy((char*) debugBuff, "\r\n") );
#endif
					// Returns The Update Page (Success)
					return "/update.html";
				}
			}
		}
	}

	// Returns The 404 Page (Fail)
	return "/404.html";
}

/**
 * @brief Handles Post  Request
 * @param[in] connection
 * @param[in] url
 * @param[in] http request
 * @param[in] http request size
 * @param[in] content size
 * @param[in] pointer to response url
 * @param[in] response url size
 * @param[in] pointer to post window
 * @return Error code
 * Note: More Information in httpd.h
 **/

err_t httpd_post_begin( void *connection, const char *uri, const char *http_request,
                        u16_t http_request_len, int content_len, char *response_uri,
                        u16_t response_uri_len, u8_t *post_auto_wnd )
{
    char referer[32];           // To Store The Page From Where Request Originate
    char contentType[25];		// To Store The Type Of Content Request Body Contains
    char header[32];			// To Store The Header Text
    char crc32[11];				// To Store The Received CRC Value

    // Clearing The Buffers
    memset(referer, '\0', sizeof(referer));
    memset(contentType, '\0', sizeof(contentType));
    memset(header, '\0', sizeof(header));
    memset(crc32, '\0', sizeof(crc32));

    // Extracting Info From Header
    extractHeaderValue(http_request, "Referer:", referer, sizeof(referer));
    extractHeaderValue(http_request, "Content-type:", contentType, sizeof(contentType));

    // Checks If Request Originated From Update Page
    if (strstr(referer, "update.html") != NULL)
    {
    	// Checks If Request Body Have Binary Data
    	if(strcmp(contentType,"application/octet-stream") == 0)
    	{
    		extractHeaderValue(http_request, "X-CRC32:", crc32, sizeof(crc32));

    		// Checks If There Is CRC Parameter
    		if(strcmp(crc32,"Not found") == 0)
    		{
    			extractHeaderValue(http_request, "State:", header, sizeof(header));

    			// Checks For Error Or False Request
    			if(strcmp(header,"Not found") == 0)
				{
    				snprintf(response_uri, response_uri_len, "/404.html");
    				return ERR_ARG;
				}
    			// Checks If Request Is To Starting Firmware Upload
    			else if(strstr(header, "starting firmware upload") != NULL)
    			{
    				// Set The State To Starting Download
    				updateState = 1;

    				// Extracting And Storing Total Number Of Chunks That Will Be Sent
    				char temp[4];
    				memset(temp, '\0', sizeof(temp));

    				temp[0] = header[25];
					temp[1] = header[26];
					temp[2] = header[27];

					firmwareChunks = (uint8_t)atoi(temp);

					// Erasing The Flash According To Firmware Size
					if(firmwareChunks > 128){
						eraseFlashSector(FLASH_SECTOR_9);
						eraseFlashSector(FLASH_SECTOR_10);
					}
					else{
						eraseFlashSector(FLASH_SECTOR_9);
					}
#ifdef SERIAL_DEBUG
					SerialDebug( strcpy((char*) debugBuff, "\r\n") );
					SerialDebug( strcpy((char*) debugBuff, "---------Update Requested\r\n") );
					SerialDebug( strcpy((char*) debugBuff, "---------Downloading Firmware\r\n") );
#endif
    				snprintf(response_uri, response_uri_len, "/update.html");
    				return ERR_OK;
    			}
    			// Checks If Request Is To Finish Firmware Upload
    			else if(strstr(header, "firmware upload completed") != NULL)
				{
    				// Set The State To Download Complete
					updateState = 3;

					// Checking For All Chunk Received
					if(firmwareChunksRecived == firmwareChunks)
					{
						// Updating Update Flags In Setting Writing To Flash & Clearing OTA Variables
						strcpy(poeSettings.update , "true");
						strcpy(poeSettings.boot , "required");
						poeSettings.size = firmwareChunks;
						writeSettingsToFlash(&poeSettings);
						firmwareChunks = 0;
						firmwareChunksRecived = 0;

					}
#ifdef SERIAL_DEBUG
					SerialDebug( strcpy((char*) debugBuff, "---------Download Completed\r\n") );
					SerialDebug( strcpy((char*) debugBuff, "\r\n") );
#endif
					snprintf(response_uri, response_uri_len, "/update.html");
					return ERR_OK;
				}
			}
    		// If There Is CRC Parameter
    		else
			{
    			// Set The State To Downloading
    			updateState = 2;

    			// Converting String CRC TO Numeric And Storing It For The Chunk
    			crc = strtoul(crc32, NULL, 10);

				snprintf(response_uri, response_uri_len, "/update.html");
				return ERR_OK;
			}
		}

        snprintf(response_uri, response_uri_len, "/update.html");
        return ERR_OK;
    }
    // Checks If Request Originated From ACl Page
    else if(strstr(referer, "acl.shtml") != NULL)
    {
    	// Checks If Request Body Have ACL Entries
    	 if (strstr(contentType, "acl-table/entries") != NULL)
    	 {
#ifdef SERIAL_DEBUG
    		 SerialDebug( strcpy((char*) debugBuff, "\r\n") );
    		 SerialDebug( strcpy((char*) debugBuff, "---------ACL Entries Submitted\r\n") );

			 SerialDebug( strcpy((char*) debugBuff, "\r\n") );
#endif
			 // Set Flag For ACL Entries Received
    		 aclFlag = true;

    		 // Extracting And Storing Total Number Of Entries That Is Sent
    		 extractHeaderValue(http_request, "Entries:", header, sizeof(header));

    		 char temp[2];
			 memset(temp, '\0', sizeof(temp));

			 temp[0] = header[0];
			 temp[1] = header[1];

		     entries = (uint8_t)atoi(temp);

    		 snprintf(response_uri, response_uri_len, "/acl.shtml");
    		 return ERR_OK;
    	 }
    }
    // Checks If Request Originated From VLAN Page
    else if(strstr(referer, "vlan.html") != NULL)
    {
    	// Checks If Request Body Have VLAN Groups
    	if (strstr(contentType, "vlan/groups") != NULL)
    	{
#ifdef SERIAL_DEBUG
    		 SerialDebug( strcpy((char*) debugBuff, "\r\n") );
			 SerialDebug( strcpy((char*) debugBuff, "---------VLAN Group Submitted\r\n") );
#endif
			 // Set Flag For VLAN Group Received
    		vlanFlag = true;

    		snprintf(response_uri, response_uri_len, "/vlan.html");
    		return ERR_OK;
    	}
    }
	snprintf(response_uri, response_uri_len, "/404.html");
	return ERR_OK;
}

/**
 * @brief Handles Post Request Data
 * @param[in] connection
 * @param[in] Structure pbuf
 * @return Error code
 * Note: More Information in httpd.h
 **/

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {

	// Checks If State Is Downloading
	if(updateState == 2)
	{
		// Extracts The Payload Into A Buffer
		uint8_t bin[p->tot_len];
		memset(bin, '\0', sizeof(bin));
		memcpy(bin,p->payload, p->tot_len);
		size_t binLength = sizeof(bin) / sizeof(bin[0]);

		// Calculate The Flash Start Address OF the Chunk Received
		uint32_t adress = FLASH_SECTOR_9_START_ADDR + (firmwareChunksRecived * 0x400);   // Note: 0x400 for 1KB Size Chunk

		firmwareChunksRecived++;  // Update Number Of Chunk Received

		// Checks If Received And Calculated CRC Matches
		if(crc == calculateCRC32(bin, binLength))
		{
			// Clear The Payload Buffer & Write The Chunk To Flash
			pbuf_free(p);
			writeFlash(adress, bin, binLength);
			return ERR_OK;
		}
		else
		{
			// Clear The Payload Buffer & Terminate The Download
			pbuf_free(p);
			updateState = 0;
			return ERR_OK;
		}
	}
	// Checks If ACl Flag True
	else if(aclFlag)
	{
		// Extracts The Payload Into A Buffer & Pass The Buffer To Respective Function
		char  data[p->tot_len];
		memset(data, '\0', sizeof(data));
		memcpy(data,p->payload, p->tot_len);

		// Handle the Error
		if(parseACLEntries(data))
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "\r\n") );
			SerialDebug( strcpy((char*) debugBuff, "---------Error Occurred While Parsing ACL Entries\r\n"));
#endif
		}
		else
		{
#ifdef SERIAL_DEBUG

			SerialDebug( strcpy((char*) debugBuff, "---------ACL Entries Set & stored\r\n") );
#endif
		}
	}
	// Checks If VLAN Flag True
	else if(vlanFlag)
	{
		// Extracts The Payload Into A Buffer & Pass The Buffer To Respective Function
		char  data[p->tot_len];
		memset(data, '\0', sizeof(data));
		memcpy(data,p->payload, p->tot_len);

		// Handle the Error
		if(parseVLAN(data))
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "\r\n") );
			SerialDebug( strcpy((char*) debugBuff, "\n---------Error Occurred While Parsing VLAN Group\r\n"));
#endif
		}
		else
		{
#ifdef SERIAL_DEBUG
			SerialDebug( strcpy((char*) debugBuff, "\r\n") );
			SerialDebug( strcpy((char*) debugBuff, "---------VLAN Group Set & stored\r\n") );
#endif
		}
	}

	pbuf_free(p);
	return ERR_OK;
}

/**
 * @brief Handles Post Request End
 * @param[in] connection
 * @param[in] response url
 * @param[in] response url size
 * Note: More Information in httpd.h
 **/

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
	// Checks If The Update State Is Between 1 & 3
	if(updateState > 0 && updateState <= 3)
	{
		// Returns Update Page To Confirm Request Success
		snprintf(response_uri, response_uri_len, "/update.html");
	}
	// Checks If ACl Flag True
	else if(aclFlag)
	{
		// Returns ACL Page To Confirm Request Success
		aclFlag = false;
		snprintf(response_uri, response_uri_len, "/acl.shtml");
	}
	// Checks If VLAN Flag True
	else if(vlanFlag)
	{
		// Returns VLAN Page To Confirm Request Success
		vlanFlag = false;
		snprintf(response_uri, response_uri_len, "/vlan.html");
	}
	else
	{
		// Returns Error Page
		snprintf(response_uri, response_uri_len, "/404.html");
	}
}

/**
 * @brief Extract values from header
 * @param[in] header
 * @param[in] filed name
 * @param[in] pointer to value
 * @param[in] size of the value
 **/

void extractHeaderValue(const char *header, const char *fieldName, char *value, size_t maxSize)
{
    const char *field = strstr(header, fieldName);

    if (field != NULL)
    {
        field += strlen(fieldName);

        while (*field == ' ' || *field == ':')
        {
            field++;
        }

        snprintf(value, maxSize, "%s", field);
    }
    else
    {
        snprintf(value, maxSize, "Not found");
    }
}

/**
 * @brief Calculate the CRC
 * @param[in] data
 * @param[in] data size
 * @return CRC
 **/

uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (int j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLYNOMIAL : 0);
        }
    }

    return ~crc;
}

/**
 * @brief Compare the firmware versions
 * @param[in] version1
 * @param[in] version2
 * @return results
 **/

uint8_t compareVersions(const char *version1, const char *version2)
{
    char *token1, *token2;
    char *saveptr1, *saveptr2;

    // Skip leading 'v' if present
    if (version1[0] == 'v') version1++;
    if (version2[0] == 'v') version2++;

    // Tokenize version strings using '.' as the delimiter
    token1 = strtok_r((char *)version1, ".", &saveptr1);
    token2 = strtok_r((char *)version2, ".", &saveptr2);

    while (token1 != NULL && token2 != NULL) {
        int part1 = atoi(token1);
        int part2 = atoi(token2);

        if (part1 < part2) {
            return 2;
        } else if (part1 > part2) {
            return 1;
        }

        // Move to the next token
        token1 = strtok_r(NULL, ".", &saveptr1);
        token2 = strtok_r(NULL, ".", &saveptr2);
    }

    // If one version string has more parts, consider it greater
    if (token1 != NULL) {
        return 1;
    } else if (token2 != NULL) {
        return 2;
    }

    // Versions are equal
    return 0;
}

/**
 * @brief Parse & process the ACL Entries
 * @param[in] data ACL entries string
 **/

poe_err parseACLEntries(char *data)
{
	poe_err ret;

	uint8_t port = 0;							// To Store The Port Number
	uint8_t auMode = 0;							// To Store The Authentication Mode
	uint8_t service = 0;						// To Store The Service On/Off
	char delimiter = '&';						// Holds The Delimiter Between Entries
	char *entry = strtok(data, &delimiter);		// Hold The Entries

	char temp[3];
	KSZ9897AdvanceFeatures sheet;

	memset(&sheet, '\0', sizeof(sheet));
	readACLSheetToFlash(&sheet, sizeof(KSZ9897AdvanceFeatures));

	// Extracts The Port Number
	extractHeaderValue(data, "port=", temp, sizeof(temp));
	port = temp[0] - '0';
	// Extract The Authentication Mode
	extractHeaderValue(data, "auth=", temp, sizeof(temp));
	auMode = temp[0] - '0';
	// Extracts The Service On/Off
	extractHeaderValue(data, "service=", temp, sizeof(temp));
	service = temp[0] - '0';

	// Loop Through The Entries
	for(int i = 1; i <= entries; i++)
	{
		// To Hold The The Entry
		ACLTableEntry tempEntry;
		memset(&tempEntry, '\0', sizeof(tempEntry));

		char *endptr;
		char *parameter_ptr;

		// Extracts The Mode
		extractHeaderValue(entry, "mode=", temp, sizeof(temp));
		tempEntry.MD = temp[0] - '0';
		// Extracts The Enable
		extractHeaderValue(entry, "compare=", temp, sizeof(temp));
		tempEntry.ENB = temp[0] - '0';
		// Extracts The Source/Destination
		extractHeaderValue(entry, "sd=", temp, sizeof(temp));
		tempEntry.SD = temp[0] - '0';
		// Extracts The Equality
		extractHeaderValue(entry, "permission=", temp, sizeof(temp));
		tempEntry.EQ = temp[0] - '0';

		// Checks If MAC Mode
		if(tempEntry.MD == LAYER2_MAC_ETHERTYPE_MATCHING)
		{
			// Checks IF EtherType Enable
			if(tempEntry.ENB != COMPARE_MAC_ONLY)
			{
				// Extracts The EtherType
				char type [5];
				extractHeaderValue(entry, "type=0x", type, sizeof(type));
				uint16_t tempType = strtoul(type, &endptr, 16);
				tempEntry.EtherType[0] = (uint8_t)((tempType >> 8) & 0xFF);
				tempEntry.EtherType[1] = (uint8_t)(tempType  & 0xFF);
			}
			// Extracts The MAC Address
			char mac [18];
			extractHeaderValue(entry, "mac=", mac, sizeof(mac));
			parameter_ptr = mac;
			tempEntry.MAC_Address[0] = (uint8_t)strtoul(strsep(&parameter_ptr, ":-"), &endptr, 16);
			tempEntry.MAC_Address[1] = (uint8_t)strtol(strsep(&parameter_ptr, ":-"), &endptr, 16);
			tempEntry.MAC_Address[2] = (uint8_t)strtol(strsep(&parameter_ptr, ":-"), &endptr, 16);
			tempEntry.MAC_Address[3] = (uint8_t)strtol(strsep(&parameter_ptr, ":-"), &endptr, 16);
			tempEntry.MAC_Address[4] = (uint8_t)strtol(strsep(&parameter_ptr, ":-"), &endptr, 16);
			tempEntry.MAC_Address[5] = (uint8_t)strtol(strsep(&parameter_ptr, ":-"), &endptr, 16);
		}
		// Checks If IP Mode
		else if(tempEntry.MD == LAYER3_IP_MATCHING)
		{
			// Checks If IP Mask Enable
			if(tempEntry.ENB != COMPARE_IPV4_SOURCE_WITHOUT_MASK)
			{
				// Extracts The IP Mask
				char mask[16];
				extractHeaderValue(entry, "mask=", mask, sizeof(mask));
				parameter_ptr = mask;
				tempEntry.IP_Mask[0] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
				tempEntry.IP_Mask[1] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
				tempEntry.IP_Mask[2] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
				tempEntry.IP_Mask[3] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
			}
			// Extracts The IP Address
			char ip[16];
			extractHeaderValue(entry, "ip=", ip, sizeof(ip));
			parameter_ptr = ip;
			tempEntry.IP_Address[0] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
			tempEntry.IP_Address[1] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
			tempEntry.IP_Address[2] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
			tempEntry.IP_Address[3] = (uint8_t)atoi(strsep(&parameter_ptr, "."));
		}
		// Checks If TCP/UDP Mode
		else if(tempEntry.MD == LAYER4_TCP_UDP_IP_MATCHING)
		{
			// Checks If IP Protocol Enable
			if(tempEntry.ENB == COMPARE_IP_PROTOCOL)
			{
				// Extracts The IP Protocol
				char protocol[3];
				extractHeaderValue(entry, "protocol=0x", protocol, sizeof(protocol));
				tempEntry.PRO = strtoul(protocol, &endptr, 16);
			}
			// Checks If TCP/UDP Port Enable
			else if(tempEntry.ENB == COMPARE_TCP_SD_PORT || tempEntry.ENB == COMPARE_UDP_SD_PORT)
			{
				// Extracts The TCP/UDP Ports
				char mPort[5];
				extractHeaderValue(entry, "maxPort=0x", mPort, sizeof(mPort));
				uint16_t tempPort = strtoul(mPort, &endptr, 16);
				tempEntry.MaxPort[0] = (uint8_t)((tempPort >> 8) & 0xFF);
				tempEntry.MaxPort[1] = (uint8_t)(tempPort  & 0xFF);

				extractHeaderValue(entry, "minPort=0x", mPort, sizeof(mPort));
				tempPort = strtoul(mPort, &endptr, 16);
				tempEntry.MinPort[0] = (uint8_t)((tempPort >> 8) & 0xFF);
				tempEntry.MinPort[1] = (uint8_t)(tempPort  & 0xFF);
			}
			// Checks If TCP Sequence Number Enable
			else if(tempEntry.ENB == COMPARE_TCP_SEQUENCE_NUMBER)
			{
				// Extracts The TCP Sequence Number
				char sNumber[9];
				extractHeaderValue(entry, "sNumber=0x", sNumber, sizeof(sNumber));
				uint32_t tempNumber = strtoul(sNumber, &endptr, 16);
				tempEntry.TCPSnumber[0] = (uint8_t)((tempNumber >> 24) & 0xFF);
				tempEntry.TCPSnumber[1] = (uint8_t)((tempNumber >> 16) & 0xFF);
				tempEntry.TCPSnumber[2] = (uint8_t)((tempNumber >> 8) & 0xFF);
				tempEntry.TCPSnumber[3] = (uint8_t)(tempNumber  & 0xFF);
			}
			// Extracting Other Required Parameter For This Mode
			extractHeaderValue(entry, "portCom=", temp, sizeof(temp));
			tempEntry.PC = temp[0] - '0';
			extractHeaderValue(entry, "flagMatch=", temp, sizeof(temp));
			tempEntry.FME = temp[0] - '0';

			extractHeaderValue(entry, "fMask=0x", temp, sizeof(temp));
			tempEntry.FMSK = strtoul(temp, &endptr, 16);
			extractHeaderValue(entry, "flag=0x", temp, sizeof(temp));
			tempEntry.FLAG = strtoul(temp, &endptr, 16);
		}

		/*****************************************************************************************************************/

		/*
		 * This Section Store Action Rule
		 * Note: Implement Your Own Logic To Define Different Action Rules
		 */

		tempEntry.FRN = i - 1; // This represent which action should be taken against this entry rule (value 0-15)

		// Modify it to set your own rule set, the rule set define which matching entries to pair against action rule
		if(i <= 8)
		{
			tempEntry.Rule_Set[0] = 0x00;
			tempEntry.Rule_Set[1] = pow(2, i -1);
		}
		if(i > 8)
		{
			tempEntry.Rule_Set[0] = pow(2, (i-9));
			tempEntry.Rule_Set[1] = 0x00;
		}

		// Making of action rule
		tempEntry.Action_Rule_PM  = 0;          // Set priority mode
		tempEntry.Action_Rule_P   = 0;          // Set priority
		tempEntry.Action_Rule_RPE = 0;          // Enable remark priority
		tempEntry.Action_Field_RP = 0;          // Set remark priority
		tempEntry.Action_Field_MM = 0;          // Set mapping mode
		tempEntry.Action_Field_Forward = 0x7F;  // Set port forward map

		/*****************************************************************************************************************/

		// Store The ACl Entry In The Sheet
		sheet.aclSheet[port - 1][i - 1] = tempEntry;

		// Writes The ACL Entry
		ret = ksz9897WriteACLTableEntry(HSPI, port, (i - 1), &tempEntry);

		// Fetch The Next Entry
		entry = strtok(NULL, &delimiter);
	}

	// Update The ACl Settings
	poeSettings.portACL[port - 1][0] = service;
	poeSettings.portACL[port - 1][1] = auMode;

	// Write Sheet To The Flash
	writeACLSheetToFlash(&sheet, sizeof(KSZ9897AdvanceFeatures));

	// Writes Updated Settings To The Flash
	writeSettingsToFlash(&poeSettings);

	// Apply The Authentication Mode & Service To The Port
	ret = ksz9897EnableACL(HSPI, port, service, auMode);

    return ret;
}

/**
 * @brief Parse & process the VLAN Groups
 * @param[in] data VLAN Group string
 **/

poe_err parseVLAN(char *data)
{
	poe_err ret;

	uint8_t port = 0;						// To Store The Port Number
	uint8_t service = 0;					// To Store The Service On/Off

	char temp[3];
	VLANGroup tempGroup;
	KSZ9897AdvanceFeatures group;

	memset(&group, '\0', sizeof(group));
	readACLSheetToFlash(&group, sizeof(KSZ9897AdvanceFeatures));

	// Extracts The Port Number
	extractHeaderValue(data, "port=", temp, sizeof(temp));
	port = temp[0] - '0';
	// Extracts The Service On/Off
	extractHeaderValue(data, "service=", temp, sizeof(temp));
	service = temp[0] - '0';

	// Loop Through The Group
	char s[7];
	for(int i = 0; i <= 5; i++)
	{
		sprintf(s, "port%d=",(i + 1));
		extractHeaderValue(data  , s, temp, sizeof(temp));
		tempGroup.vlanPortGroup[i] = temp[0] - '0';
	}
	// Checks The Service Is On/Off
	if(service == 1)
	{
		// Set The VLAN Group
		ret = ksz9897WriteVLANGroup(HSPI, port, &tempGroup);
	}

	tempGroup.vlanService = service;
	group.vlanGroups[port - 1] = tempGroup;

	// Write Sheet To The Flash
	ret = writeACLSheetToFlash(&group, sizeof(KSZ9897AdvanceFeatures));

	return ret;
}

/**
 * @brief Initialize the server and set respective handlers
 **/

void http_server_init (void)
{
	// Start The HTTPD Service
	httpd_init();

	// Set CGI Handlers To Handle CGI Request
	CGI_TAB[0] = LOGIN_CGI;
	CGI_TAB[1] = SETTING_CGI;
	CGI_TAB[2] = UPDATE_PAGE_CGI;
	CGI_TAB[3] = UPDATE_REQUEST_CGI;
	CGI_TAB[4] = RESET_CGI;
	http_set_cgi_handlers(CGI_TAB, 5);

	// Set SSI Handler That Update Status Page
	http_set_ssi_handler(ssi_handler, (char const**) TAGS, 20);

}
