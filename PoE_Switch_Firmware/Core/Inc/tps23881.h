/*
 ====================================================================================================
 * File:        tps23881.h
 * Author:      Hamas Saeed
 * Version:     Rev_1.0.0
 * Date:        Dec 05, 2023
 * Brief:       This file contains the driver for TI TPS23881 POE SOC.
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

#ifndef INC_TPS23881_H_
#define INC_TPS23881_H_

#include "main.h"
#include "sram_code.h"

/* PSE info buffers */
extern uint8_t tpsPortsStatusBuffer[5];
extern uint8_t tpsPortsDetectionBuffer[5];
extern uint8_t tpsPortsClassificationBuffer[5];
extern uint8_t tpsPortsPowerStatus[5];
extern float   tpsPortscurrent[5];
extern float   tpsPortsVoltage[5];

/* TPS23881 I2C Address */
#define TPS23881_I2C_TIMEOUT_DELAY       100               // Delay in ms
#define TPS23881_I2C_ADDR_GLOBAL         (0x7F << 1)       // Global I2C Address - 7 bit (P.26)
#define TPS23881_I2C_ADDR_1              (0x20 << 1)       // I2C Address - 7 bit (P.57)
#define TPS23881_I2C_ADDR_2              (0x21 << 1)       // I2C Address - 7 bit (P.57)

#define DEVICE_ADDR_A                    1
#define DEVICE_ADDR_B                    2
#define DEVICE_ADDR_GLOBAL               0

/* TPS23881 Configurations */
#define NUM_OF_TPS23881                  1                 // Number of TPS23881 devices
#define NUM_OF_QUARD                     2                 // Number of quards
#define NUM_OF_CHANNEL                   4                 // Number of channels per quard
#define PRINT_STATUS                     1
#define DETAILED_STATUS                  1
#define PARITY_EN                        1

/* TPS23881 Constants */
#define I_STEP                           0.00007019
#define V_STEP                           0.003662

/* Related Macros */
#define DETECT                           0x0F
#define CLASS                            0xF0
#define CLASS_SHIFT                      4
#define DETECT_SHIFT                     0
#define GET_DETECT(x)                    (x & DETECT)
#define GET_CLASS(x)                     (x >> CLASS_SHIFT)

#define PEC                              0x01
#define PGC                              0x02
#define DISF                             0x04
#define DETC                             0x08
#define CLASC                            0x10
#define IFAULT                           0x20
#define INRF                             0x40
#define SUPF                             0x80

/* Power & Other Commands */
#define TPS23881_POWER_STATUS_COMMAND                        0x10
#define TPS23881_POWER_ENABLE_COMMAND                        0x19
#define TPS23881_POWER_EVENT_CLEAR_COMMAND                   0x03

#define TPS23881_DISCONNECT_ENABLE_COMMAND                   0x13
#define TPS23881_4PWIRED_POWER_ALLOCATION_CONFIG_COMMAND     0x29
#define TPS23881_DETECT_CLASS_ENABLE_COMMAND                 0x14
#define TPS23881_DETECT_CLASS_RESTART_COMMAND                0x18

#define TPS23881_OPERATING_MODE_COMMAND                      0x12

#define TPS23881_INTERRUPT_COMMAND                           0x00
#define TPS23881_INTERRUPT_MASK_COMMAND                      0x01

#define TPS23881_DETECTION_EVENT_CLEAR_COMMAND               0x05
#define TPS23881_FAULT_EVENT_CLEAR_COMMAND                   0x07
#define TPS23881_START_LIMIT_EVENT_CLEAR_COMMAND             0x09
#define TPS23881_SUPPLY_EVENT_CLEAR_COMMAND                  0x0B

/* Channel Status Commands */
#define TPS23881_CHANNEL_1_STATUS_COMMAND                    0x0C
#define TPS23881_CHANNEL_2_STATUS_COMMAND                    0x0D
#define TPS23881_CHANNEL_3_STATUS_COMMAND                    0x0E
#define TPS23881_CHANNEL_4_STATUS_COMMAND                    0x0F

/* Channel Current Commands */
#define TPS23881_CHANNEL_1_CURRENT_COMMAND                   0x30
#define TPS23881_CHANNEL_2_CURRENT_COMMAND                   0x34
#define TPS23881_CHANNEL_3_CURRENT_COMMAND                   0x38
#define TPS23881_CHANNEL_4_CURRENT_COMMAND                   0x3C

/* Channel Voltage Commands */
#define TPS23881_CHANNEL_1_VOLTAGE_COMMAND                   0x32
#define TPS23881_CHANNEL_2_VOLTAGE_COMMAND                   0x36
#define TPS23881_CHANNEL_3_VOLTAGE_COMMAND                   0x3A
#define TPS23881_CHANNEL_4_VOLTAGE_COMMAND                   0x3E

/* Device Command */
#define TPS23881_DEVICE_ID_COMMAND                           0x43

/* Firmware Command */
#define TPS23881_FIRMWARE_REVISION_COMMAND                   0x41

/* SRAM and Parity Addresses */
#define TPS23881_SRAM_CONTROL_COMMAND                         0x60
#define TPS23881_SRAM_DATA_COMMAND                            0x61

#define TPS23881_SRAM_START_ADDRESS_COMMAND                   0x62
#define TPS23881_SRAM_START_ADDRESS_LSB_COMMAND               0x62
#define TPS23881_SRAM_START_ADDRESS_MSB_COMMAND               0x63

#define TPS23881_RAM_PREP_COMMAND_1                           0x1D
#define TPS23881_RAM_PREP_COMMAND_2                           0xD7
#define TPS23881_RAM_PREP_COMMAND_3                           0x91
#define TPS23881_RAM_PREP_COMMAND_4                           0x90

/* Operating mode for each of the ports (off, manual, semi-auto, auto) */
typedef enum {
  OPERATING_MODE_OFF                                    = 0x0,  // Off; no detection or classifications
  OPERATING_MODE_DIAGNOSTIC                             = 0x1,  // Diagnostic
  OPERATING_MODE_SEMI_AUTO                              = 0x2,  // Semi-auto, automatic detection and classification (if enabled), but no automatic power on
  OPERATING_MODE_AUTO                                   = 0x3   // Auto, automatic detection, classification, and power on
} TPS238x_Operating_Modes_t;

/* Power enable/disable */
typedef enum {
  Disable                                               = 0,
  Enable                                                = 1
} TPS23881PortPower;

/* Classification status */
typedef enum {
  CLASS_UNKNOWN                                         = 0x0,   // Unknown - invalid
  CLASS_1                                               = 0x1,   // Class 1
  CLASS_2                                               = 0x2,   // Class 2
  CLASS_3                                               = 0x3,   // Class 3
  CLASS_4                                               = 0x4,   // Class 4
  CLASS_0                                               = 0x6,   // Class 0
  CLASS_OVERCURRENT                                     = 0x7,   // Over current - invalid
  CLASS_5_4P_SINGLE                                     = 0x8,   // Class 5
  CLASS_6_4P_SINGLE                                     = 0x9,   // Class 6
  CLASS_7_4P_SINGLE                                     = 0xA,   // Class 7
  CLASS_8_4P_SINGLE                                     = 0xB,   // Class 8
  CLASS_4PLUS_TYPE1                                     = 0xC,   // Class 4+
  CLASS5_4P_DUAL                                        = 0xD,   // Class 5 dual
  CLASS_MISMATCH                                        = 0xF
} TPS2381_Classification_Status_t;

/* Detection status */
typedef enum    {
  DETECT_UNKNOWN                                        = 0x0,  // Unknown - invalid
  DETECT_SHORT_CIRCUIT                                  = 0x1,  // Short circuit (<1.8 kOhm) - invalid
  DETECT_RESIST_LOW                                     = 0x3,  // Resistance too low - invalid
  DETECT_RESIST_VALID                                   = 0x4,  // Resistance valid
  DETECT_RESIST_HIGH                                    = 0x5,  // Resistance too high - invalid
  DETECT_OPEN_CIRCUIT                                   = 0x6,  // Open circuit - invalid
  DETECT_MOSFET_FAULT                                   = 0xE   // MOSFET Fault - invalid
} TPS2381_Detection_Status_t;

/* Port Power status */
typedef struct {
    uint8_t Channel_1_Power_Status                      : 1;
    uint8_t Channel_2_Power_Status                      : 1;
    uint8_t Channel_3_Power_Status                      : 1;
    uint8_t Channel_4_Power_Status                      : 1;
    uint8_t Channel_1_Power_Good                        : 1;
    uint8_t Channel_2_Power_Good                        : 1;
    uint8_t Channel_3_Power_Good                        : 1;
    uint8_t Channel_4_Power_Good                        : 1;
} TPS23881PortPowerStatus_t;

/* Power on/off */
typedef struct {
    uint8_t Channel_1_Power_ON                          : 1;
    uint8_t Channel_2_Power_ON                          : 1;
    uint8_t Channel_3_Power_ON                          : 1;
    uint8_t Channel_4_Power_ON                          : 1;
    uint8_t Channel_1_Power_OFF                         : 1;
    uint8_t Channel_2_Power_OFF                         : 1;
    uint8_t Channel_3_Power_OFF                         : 1;
    uint8_t Channel_4_Power_OFF                         : 1;
} TPS23881PortPowerEnable_t;

/* Power allocation */
typedef struct {
    uint8_t MCnn_0                                      : 1;
    uint8_t MCnn_1                                      : 1;
    uint8_t MCnn_2                                      : 1;
    uint8_t _4PWnn                                      : 1;
} TPS23881PortPowerAllocation_t;

/* Supply Event Register */
typedef struct {
    unsigned char RAMFLT_SRAM_Fault_Event               : 1;  // SRAM memory fault
    unsigned char OSS_Event                             : 1;  // OSS event has happened
    unsigned char Four_C1_C2_PCUT_Event                 : 1;  // 4P PCUT has happened on channel 12
    unsigned char Four_C3_C4_PCUT_Event                 : 1;  // 4P PCUT has happened on pair 12
    unsigned char VPUV_VPower_Undervoltage_Event        : 1;  // VPWR Under voltage
    unsigned char VDWRN_Vdd_UVLO_Warining_Event         : 1;  // VDD falls below UVLO under the UVLO warning threshold
    unsigned char VDUV_Vdd_UVLO_Event                   : 1;  // VDD UVLO Occurred. Power on reset happened
    unsigned char TSD_Thermal_Shutdown_Event            : 1;  // Thermal shutdown occurred
} TPS2381_Supply_Event_4PPCUT_Register_t;

/* Inrush/ILIM Event Register */
typedef struct {
    uint8_t INR1_Inrush_Fault_Event_Port_1              : 1;  // Inrush fault occurred at port 1
    uint8_t INR2_Inrush_Fault_Event_Port_2              : 1;  // Inrush fault occurred at port 2
    uint8_t INR3_Inrush_Fault_Event_Port_3              : 1;  // Inrush fault occurred at port 3
    uint8_t INR4_Inrush_Fault_Event_Port_4              : 1;  // Inrush fault occurred at port 4
    uint8_t ILIM1_Limit_Output_Current_Event_Port_1     : 1;  // ILIM fault occurred at port 1
    uint8_t ILIM2_Limit_Output_Current_Event_Port_2     : 1;  // ILIM fault occurred at port 2
    uint8_t ILIM3_Limit_Output_Current_Event_Port_3     : 1;  // ILIM fault occurred at port 3
    uint8_t ILIM4_Limit_Output_Current_Event_Port_4     : 1;  // ILIM fault occurred at port 4
} TPS2381_Inrush_ILIM_Event_Register_t;

/* Fault Event Register */
typedef struct {
    uint8_t PCUT1_PCUT_Fault_Event_Port_1               : 1;  // ICUT fault occurred at port 1
    uint8_t PCUT2_PCUT_Fault_Event_Port_2               : 1;  // ICUT fault occurred at port 2
    uint8_t PCUT3_PCUT_Fault_Event_Port_3               : 1;  // ICUT fault occurred at port 3
    uint8_t PCUT4_PCUT_Fault_Event_Port_4               : 1;  // ICUT fault occurred at port 4
    uint8_t DISF1_Disconnect_Event_Port_1               : 1;  // Disconnect event occurred at port 1
    uint8_t DISF2_Disconnect_Event_Port_2               : 1;  // Disconnect event occurred at port 2
    uint8_t DISF3_Disconnect_Event_Port_3               : 1;  // Disconnect event occurred at port 3
    uint8_t DISF4_Disconnect_Event_Port_4               : 1;  // Disconnect event occurred at port 4
} TPS2381_Fault_Event_Register_t;

/* Detection Event Register */
typedef struct {
    uint8_t DETC1_Detection_Cycle_Event_Channel_1       : 1;  // Detection cycle occurred on port 1
    uint8_t DETC2_Detection_Cycle_Event_Channel_2       : 1;  // Detection cycle occurred on port 2
    uint8_t DETC3_Detection_Cycle_Event_Channel_3       : 1;  // Detection cycle occurred on port 3
    uint8_t DETC4_Detection_Cycle_Event_Channel_4       : 1;  // Detection cycle occurred on port 4
    uint8_t CLSC1_Classification_Cycle_Event_Channel_1  : 1;  // Classification cycle occurred on port 1
    uint8_t CLSC2_Classification_Cycle_Event_Channel_2  : 1;  // Classification cycle occurred on port 2
    uint8_t CLSC3_Classification_Cycle_Event_Channel_3  : 1;  // Classification cycle occurred on port 3
    uint8_t CLSC4_Classification_Cycle_Event_Channel_4  : 1;  // Classification cycle occurred on port 4
} TPS2381_Detection_Event_Register_t;

/* Power Event Register */
typedef struct {
    uint8_t PEC1_Power_Enable_Event_Port_1              : 1;  // Change to power enable status for port 1
    uint8_t PEC2_Power_Enable_Event_Port_2              : 1;  // Change to power enable status for port 2
    uint8_t PEC3_Power_Enable_Event_Port_3              : 1;  // Change to power enable status for port 3
    uint8_t PEC4_Power_Enable_Event_Port_4              : 1;  // Change to power enable status for port 4
    uint8_t PGC1_Power_Good_Event_Port_1                : 1;  // Change to power good status for port 1
    uint8_t PGC2_Power_Good_Event_Port_2                : 1;  // Change to power good status for port 2
    uint8_t PGC3_Power_Good_Event_Port_3                : 1;  // Change to power good status for port 3
    uint8_t PGC4_Power_Good_Event_Port_4                : 1;  // Change to power good status for port 4
} TPS2381_Power_Event_Register_t;

/* Interrupt Mask Register */
typedef struct {
    uint8_t PEMSK_Power_Enable_Unmask                   : 1;  // Enable power enable interrupts
    uint8_t PGMSK_Power_Good_Unmask                     : 1;  // Enable power good interrupts
    uint8_t DIMSK_Disconnect_Unmask                     : 1;  // Enable disconnect event interrupts
    uint8_t DEMSK_Detection_Cycle_Unmask                : 1;  // Enable detection cycle event interrupts
    uint8_t CLMSK_Classificiation_Cycle_Unmask          : 1;  // Enable classification cycle event interrupts
    uint8_t IFMSK_IFAULT_Unmask                         : 1;  // Enable ICUT or OLIM fault interrupts
    uint8_t INMSK_Inrush_Fault_Unmask                   : 1;  // Enable Inrush fault interrupts
    uint8_t SUMSK_Supply_Event_Fault_Unmask             : 1;  // Enable supply event fault interrupts
} TPS2381_Interrupt_Mask_Register_t;

/* Interrupt Register */
typedef struct {
    unsigned char PEC_Power_Enable_Change               : 1;   // Indicates a power enable status change occurred on at least one port
    unsigned char PGC_Power_Good_Change                 : 1;   // Indicates a power good change occurred on at least one port
    unsigned char DISF_Disconnect_Event                 : 1;   // Indicates a disconnect event occurred on at least one port
    unsigned char DETC_Detection_Cycle                  : 1;   // Indicates at least one detection cycle occurred on at least one port
    unsigned char CLASC_Classification_Cycle            : 1;   // Indicates at least one classification cycle occurred on at least one port
    unsigned char IFAULT_PCUT_ILIM_Fault                : 1;   // Indicates that an ICUT or ILIM fault occurred on at least one port
    unsigned char INRF_Inrush_Fault                     : 1;   // Indicates that an Inrush fault occurred on at least one port
    unsigned char SUPF_Supply_Event_Fault               : 1;   // Indicates that a supply event fault occurred
} TPS2381_Interrupt_Register_t;


/* Port detection status */
typedef struct {
    unsigned char PORT_1_Connected                      : 1;
    unsigned char PORT_2_Connected                      : 1;
    unsigned char PORT_3_Connected                      : 1;
    unsigned char PORT_4_Connected                      : 1;
    unsigned char PORT_5_Connected                      : 1;
    unsigned char PORT_6_Connected                      : 1;
    unsigned char PORT_7_Connected                      : 1;
    unsigned char PORT_8_Connected                      : 1;
} TPS2381_Port_Connected;


/* TPS23881 STRUCT */
typedef struct {
    I2C_HandleTypeDef *i2chandle;                              // I2C handle

    uint8_t initStatus                                  : 1;   // Initialization Status
    uint8_t interruptPin                                : 1;   // Hardware Interrupt Pin

    TPS2381_Interrupt_Register_t                        interruptStatusA, interruptStatusB;
    TPS2381_Power_Event_Register_t                      powerEventStatusA, powerEventStatusB;
    TPS2381_Fault_Event_Register_t                      faultEventStatusA, faultEventStatusB;
    TPS2381_Detection_Event_Register_t                  detectionEventStatusA, detectionEventStatusB;
    TPS2381_Inrush_ILIM_Event_Register_t                inrushIlimEventStatusA, inrushIlimEventStatusB;
    TPS2381_Supply_Event_4PPCUT_Register_t              supplyEventStatusA, supplyEventStatusB;

    TPS2381_Classification_Status_t                     classificationStatusA, classificationStatusB;
    TPS2381_Detection_Status_t                          detectionStatusA, detectionStatusB;

    TPS2381_Port_Connected                              portConnectionStatus;

    uint8_t sysPortNum;
    uint8_t portNum1;
    uint8_t portNum2;
    uint8_t portNum3;
    uint8_t portNum4;
    uint8_t portNum5;
    uint8_t portNum6;
    uint8_t portNum7;
    uint8_t portNum8;
} TPS23881;


/* Function Prototypes */
poe_err TPS23881_init(TPS23881 *dev, I2C_HandleTypeDef *i2chandle, TPS238x_Operating_Modes_t opMode);
poe_err TPS23881_UpdateSRAMCode(TPS23881 *dev);
poe_err TPS23881_UpdateSRAMCodeSafe(TPS23881 *dev);
poe_err TPS23881_ReadRegister(TPS23881 *dev, uint8_t reg, uint8_t *data, uint8_t devAdd);
poe_err TPS23881_WriteRegister(TPS23881 *dev, uint8_t reg, uint8_t *data, uint8_t devAdd);
poe_err TPS23881_GetDeviceInterruptStatus(TPS23881 *dev, uint8_t devAdd);
poe_err TPS23881_GetAndClearDevicePowerEventStatus(TPS23881 *dev, uint8_t devAdd);
poe_err TPS23881_GetAndClearDeviceDetectionEventStatus(TPS23881 *dev, uint8_t devAdd);
poe_err TPS23881_GetAndClearDeviceFaultEventStatus(TPS23881 *dev, uint8_t devAdd);
poe_err TPS23881_GetAndClearDeviceInrushEventStatus(TPS23881 *dev, uint8_t devAdd);
poe_err TPS23881_GetAndClearDevicePowerSypplyEventStatus(TPS23881 *dev, uint8_t devAdd);
poe_err GetAllInterruptStatus(TPS23881 *dev, uint8_t devAdd);
poe_err TPS23881_GetChannelDiscoveryAndClass(TPS23881 *dev, uint8_t statusCommand, uint8_t devAdd);
uint8_t TPS23881_InterruptHandler(TPS23881 *PSE, uint8_t devAd);
poe_err TPS23881_PortDetectStatus(TPS23881 *PSE, uint8_t statusCommand, uint8_t port, uint8_t devAd);
poe_err TPS23881_PortClassStatus(TPS23881 *PSE, uint8_t port);
uint8_t TPS23881_SetPortClass(TPS23881 *PSE, uint8_t class);
poe_err TPS23881_AssignPortClass(TPS23881 *PSE, uint8_t port, uint8_t class, uint8_t devAd);
poe_err TPS23881_PowerEnable(TPS23881 *PSE, uint8_t port, uint8_t enable, uint8_t devAd);
poe_err TPS23881_PowerStatus(TPS23881 *PSE, uint8_t devAd);
poe_err TPS23881_ReadPortsVoltages(TPS23881 *PSE, uint8_t devAd);
poe_err TPS23881_ReadPortsCurrents(TPS23881 *PSE, uint8_t devAd);
uint8_t TPS23881_PortAutoRecovery(TPS23881 *PSE, uint8_t devAd);
poe_err TPS23881_RestartDetectClass(TPS23881 *PSE, uint8_t port, uint8_t devAd);

#endif /* INC_TPS23881_H_ */
