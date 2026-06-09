/*
 * File:    SrvIntCMD.h
 * Author:  GMA
 * Data:    11.05.2023+
 */

#ifndef __SRVINTCMD_H__
#define __SRVINTCMD_H__

// START_BYTE = 0x55
#define START_BYTE                  (unsigned char)0x55

//------------------------------------------------------------------------------
// Firmware version number, saved in program flash of microcontroller
#define FIRMWARE_REVISION           0x5

#define MSB_DEVICE_ID_ACCESS_KEY    (unsigned char)0x75
#define MIDB_DEVICE_ID_ACCESS_KEY   (unsigned char)0xF3
#define LSB_DEVICE_ID_ACCESS_KEY    (unsigned char)0x19

// Constants for device addressing in interface SrvInt
#define DEVICE_ADDRESS				(unsigned char)0x10
#define NULL_ADDRESS				(unsigned char)0x00
#define BROADCAST_ADDRESS			(unsigned char)0x55
#define MASTER_ADDRESS				(unsigned char)0xCA

// Commands list ...............................................................
// _______________________Sended from uC to PC__________________________________|_____Sended from PC to uC______
#define CMD_GO_TO_BOOT_MODE         (unsigned char)0x77                         // Operands absent
//									
// Non answer from uC
////////////////////////////////////////////////////////////////////////////////
#define CMD_PING                    (unsigned char)0x81                         // Operands absent
//									
// Answer from uC:  Op1 = LAST_ERROR
////////////////////////////////////////////////////////////////////////////////
#define CMD_HW_RESET				(unsigned char)0x82                         // Operands absent
//									
// Answer from uC:  Op1 = LAST_ERROR
////////////////////////////////////////////////////////////////////////////////
#define CMD_SW_RESET				(unsigned char)0x83                         // Operands absent
//
// Answer from uC:  Op1 = LAST_ERROR
////////////////////////////////////////////////////////////////////////////////
#define CMD_GET_ERROR				(unsigned char)0x84                         // Op1 = ErrorPosition
//
// Answer from uC:  Op1 = ErrorPosition
//                  Op2 = ErrorValue
//                  Op3 = LAST_ERROR
////////////////////////////////////////////////////////////////////////////////
#define CMD_ZEROIZE_ERRORS                                  (unsigned char)0x85 // Operands absent
//									
// Answer from uC: Op1 = LAST_ERROR
////////////////////////////////////////////////////////////////////////////////
#define CMD_SET_PARAM                                       (unsigned char)0x86
//	Op1 =
#define SET_PARAM_DEVICE                                    (unsigned char)0x00 // <-- Op1
#define SET_PARAM_HBIT                                      (unsigned char)0xB0 // <-- Op1
#define SET_PARAM_GOTO                                      (unsigned char)0xB1 // <-- Op1
#define SET_PARAM_ENGINES                                   (unsigned char)0xB2 // <-- Op1
//#define SET_PARAM_VOLTAGES                                  (unsigned char)0xB3 // <-- Op1
#define SET_PARAM_POWER                                     (unsigned char)0xB4 // <-- Op1
#define SET_PARAM_RGB_LEDS                                  (unsigned char)0xB5 // <-- Op1
#define SET_PARAM_SNS                                       (unsigned char)0xB6 // <-- Op1
#define SET_PARAM_GERCON                                    (unsigned char)0xB7 // <-- Op1
#define SET_PARAM_OPT_DIAG                                  (unsigned char)0xB8 // <-- Op1
#define SET_PARAM_PBTN                                      (unsigned char)0xB9 // <-- Op1

// <SET_PARAM_DEVICE>...........................................................
#define SET_PARAM_DEVICE_SET_ID                             (unsigned char)0x01 // Op3 = MSB_DEVICE_ID_ACCESS_KEY
// 	Answer from uC: Op1 = SET_PARAM_DEVICE                                      // Op4 = MIDB_DEVICE_ID_ACCESS_KEY
//                  Op2 = SET_PARAM_DEVICE_GET_ID                               // Op5 = LSB_DEVICE_ID_ACCESS_KEY
//                  Op3 = LAST_ERROR                                            // Op6:9 = ID Value
// <SET_PARAM_HBIT>.............................................................
#define SET_PARAM_HBIT_MODE                                 (unsigned char)0x01 // Op3 = Mode
// Answer from uC: 	Op1 = SET_PARAM_HBIT                                                                  
//                  Op2 = SET_PARAM_HBIT_MODE  
//                  Op3 = Mode
//                  Op4 = LAST_ERROR     
#define SET_PARAM_HBIT_REP_FACT                             (unsigned char)0x02 // Op3 = Repetition Factor
// Answer from uC: 	Op1 = SET_PARAM_HBIT                                                                   
//                  Op2 = SET_PARAM_HBIT_MODE 
//                  Op3 = Repetition Factor
//                  Op4 = LAST_ERROR    
// <SET_PARAM_GOTO>.............................................................// Op1 = SET_PARAM_GOTO
// Op2 = ENGINE_NUMBER                                                          // Op2 = ENGINE_NUMBER
// Op3 = see below OPCODE                                                       // Op3 = OPCODE
#define SET_PARAM_GOTO_OPCODE_GOTO_N_TICKS                  (unsigned char)0x00 // Op4:Op11 = Ticks (Int64)  
                                                                                // if dual mode than Op12 = Engine b Number
                                                                                // Op13 = OPCODE = Op3 (not have influence)
                                                                                // Op14:Op21 = Ticks (Int64)
#define SET_PARAM_GOTO_OPCODE_GOTO_ABS_POSITION             (unsigned char)0x01 // Op4:Op11 = Position in Ticks (Int64)
                                                                                // if dual mode than Op12 = Engine b Number
                                                                                // Op13 = OPCODE = Op3 (not have influence)
                                                                                // Op14:Op21 = Position in Ticks (Int64) for Eb
#define SET_PARAM_GOTO_OPCODE_GOTO_ZERO_POSITION            (unsigned char)0x02 // Op4 = Switch Select, Op5,... absent
        // Op4 = see below
        #define SW1_IS_ZERO                                 (unsigned char)0x00
        #define SW2_IS_ZERO                                 (unsigned char)0x01

#define SET_PARAM_GOTO_OPCODE_EMERGENCY_STOP                (unsigned char)0x03 // Op4,... absent
                                                                                // if dual mode than
                                                                                // Op4 = Eb number
#define SET_PARAM_GOTO_OPCODE_UNCONTROLLED_GOTO_N           (unsigned char)0x04 // Op4:Op11 = Ticks (Int64)
                                                                                // if dual mode than Op12 = Engine b Number
                                                                                // Op13 = OPCODE = Op3 (not have influence)
                                                                                // Op14:Op21 = Ticks (Int64) for Eb
#define SET_PARAM_GOTO_OPCODE_SPEED_SET                     (unsigned char)0x05 // Op4:Op7 = Speed Index (Int32)
                                                                                // if dual mode than
                                                                                // Op8 = Engine Number of Eb
                                                                                // Op9 = OPCODE = Op3 (not have influence)
                                                                                // Op10:Op13 = Speed Index (Int32) of Eb
#define SET_PARAM_GOTO_OPCODE_SMOOTH_BREAKING               (unsigned char)0x06 // Op4,... absent
                                                                                // if dual mode than
                                                                                // Op4 = Eb number
#define SET_PARAM_GOTO_OPCODE_UNC_GOTO_N_WITHOUT_OPTS       (unsigned char)0xF7 // Op4:Op11 = Ticks (Int64)
                                                                                // if dual mode than Op12 = Engine b Number
                                                                                // Op13 = OPCODE = Op3 (not have influence)
                                                                                // Op14:Op21 = Ticks (Int64) for Eb
#define SET_PARAM_GOTO_OPCODE_SPEED_SET_WITHOUT_OPTS        (unsigned char)0xF5 // Op4:Op7 = Speed Index (Int32)
                                                                                // if dual mode than Op12 = Engine b Number
                                                                                // Op13 = OPCODE = Op3 (not have influence)
                                                                                // Op14:Op21 = Ticks (Int64) for Eb
#define SET_PARAM_GOTO_OPCODE_MIXMODE_ON                    (unsigned char)0x56 // Op4:Op7 = Number Of Cycles,
                                                                                // Op8:Op11 = Cycles length in ticks
#define SET_PARAM_GOTO_OPCODE_TIME_MIXMODE_ON               (unsigned char)0x57 // Op4:Op7 = Time in seconds,
                                                                                // Op8:Op11 = Cycles length in ticks

#define SET_PARAM_GOTO_OPCODE_MIXMODE_OFF                   (unsigned char)0x58 // Op4,... absent
#define SET_PARAM_GOTO_OPCODE_MIXMODE_SMOOTH_BREAKING       (unsigned char)0x59 // Op4,... absent
#define SET_PARAM_GOTO_OPCODE_ACCEL_AND_STANDBY_CURRENTS    (unsigned char)0x5A // Op4 = Acceleration Current, Op5 = Standby Current, Op6,... absent
#define SET_PARAM_GOTO_OPCODE_ACCEL_CHANGE_PAUSE            (unsigned char)0x5B // Op4 = Acceleration Current Change Pause, Op5,... absent
// Answer from uC: 	Op1 = SET_PARAM_GOTO
//                  Op2 = ENGINE_NUMBER                                                                   
//                  Op3 = OPCODE 
//                  Op4 = ENGINE_STATUS
//                  Op5 = LAST_ERROR   
// <SET_PARAM_ENGINES>..........................................................// Op1 = SET_PARAM_ENGINES
// Op2 = ENGINE_NUMBER                                                          // Op2 = ENGINE_NUMBER
// Op3 = see below PCODE                                                        // Op3 = PCODE
#define SET_PARAM_ENGINES_PCODE_MAX_SPEED                   (unsigned char)0x00 // Op4:Op7 = Max Speed Index (Int32)
#define SET_PARAM_ENGINES_PCODE_REPEATABILITY               (unsigned char)0x03 // Op4 = Repeatability (UInt8)
#define SET_PARAM_ENGINES_PCODE_MICROSTEP_MODE              (unsigned char)0x04 // Op4 = Mode (UInt8)
#define SET_PARAM_ENGINES_PCODE_HOME_POSITION               (unsigned char)0x05 // Op4:Op11 = Position (Int64)
#define SET_PARAM_ENGINES_PCODE_F0                          (unsigned char)0x06 // Op4:Op11 = F0 (Double64)
#define SET_PARAM_ENGINES_PCODE_FMAX                        (unsigned char)0x07 // Op4:Op11 = FMAX (Double64)
#define SET_PARAM_ENGINES_PCODE_DFMAX                       (unsigned char)0x08 // Op4:Op11 = DFMAX (Double64)
#define SET_PARAM_ENGINES_PCODE_SET_POSITION                (unsigned char)0x1C // Op4:Op11 = Position (Int64)
#define SET_PARAM_ENGINES_PCODE_SAVE_POSITION               (unsigned char)0x0C // Op4,... absent
#define SET_PARAM_ENGINES_PCODE_LOAD_POSITION               (unsigned char)0x0D // Op4,... absent
#define SET_PARAM_ENGINES_PCODE_ENGINE_ON                   (unsigned char)0x1A // Op4,... absent
#define SET_PARAM_ENGINES_PCODE_ENGINE_OFF                  (unsigned char)0x1B // Op4,... absent
#define SET_PARAM_ENGINES_PCODE_DRIVER_WRITE                (unsigned char)0x30 // Op4:Op7 = TMC2660C Driver Data
#define SET_PARAM_ENGINES_PCODE_FACTORY_DEFAULT             (unsigned char)0xF0 // Op4,... absent
// Answer from uC: 	Op1 = SET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER                                                                  
//                  Op3 = PCODE 
//                  Op4 = ENGINE_STATUS
//                  Op5 = LAST_ERROR
// <SET_PARAM_POWER>............................................................// Op1 = SET_PARAM_POWER
// Op2 = see below PCODE                                                        
#define SET_PARAM_POWER_COOLER                              (unsigned char)0x01 // Op3:Op4 = PWM Value
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_COOLER                                                                  
//                  Op3 = LAST_ERROR
#define SET_PARAM_POWER_LEDS_LINE                           (unsigned char)0x02 // Op3:Op4 = PWM Value
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_LEDS_LINE                                                                  
//                  Op3 = LAST_ERROR
#define SET_PARAM_POWER_UV_LAMP                             (unsigned char)0x03 // Op3 = 0 Lamp is OFF, !=0 Lamp is ON
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_UV_LAMP                                                                  
//                  Op3 = LAST_ERROR
#define SET_PARAM_POWER_BUZZER                              (unsigned char)0x04 // Op3 = 0 Buzzer is OFF, !=0 Buzzer is ON
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_BUZZER                                                                  
//                  Op3 = LAST_ERROR
#define SET_PARAM_POWER_SHDN                                (unsigned char)0x05 // Op3 = 0 SHDN=0, !=0 SHDN=1
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_SHDN                                                                  
//                  Op3 = LAST_ERROR
#define SET_PARAM_POWER_WDT_VALUE                           (unsigned char)0x06 // Op3:Op6 = WDT value
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_WDT_VALUE                                                                  
//                  Op3 = LAST_ERROR
#define SET_PARAM_POWER_TURN_OFF                            (unsigned char)0x07 // Op3,.... absent
// Answer from uC: 	Op1 = SET_PARAM_POWER
//                  Op2 = SET_PARAM_POWER_TURN_OFF                                                               
//                  Op3 = LAST_ERROR
// <SET_PARAM_RGB_LEDS>.........................................................// Op1 = SET_PARAM_RGB_LEDS
// Op2 = see below
#define SET_PARAM_RGB_LEDS_ONOFF                            (unsigned char)0x01 // Op3:Op6 = (Int32) RGBLinesType
// Answer from uC: 	Op1 = SET_PARAM_RGB_LEDS
//                  Op2 = SET_PARAM_RGB_LEDS_ONOFF
//                  Op3 = LAST_ERROR
// <SET_PARAM_SNS>.........................................................// Op1 = SET_PARAM_VOLTAGES
// Op2 = see below
#define SET_PARAM_SNS_TRANS_FUNCT                           (unsigned char)0x01 // Op3 = Cmax (Int8)
// Answer from uC: 	Op1 = SET_PARAM_VOLTAGES                                    // Op4:Op11 = Va (Double64) in mV
//                  Op2 = SET_PARAM_VOLTAGES_TRANS_FUNCT                        // Op12:Op19 = Vb (Double64) in mV
//                  Op3 = LAST_ERROR
#define SET_PARAM_SNS_DEBUG_MODE                            (unsigned char)0x02 // Op3 = Mode (1 - on, 0 - off)
// Answer from uC: 	Op1 = SET_PARAM_VOLTAGES
//                  Op2 = SET_PARAM_VOLTAGES_DEBUG_MODE
//                  Op3 = LAST_ERROR
// <SET_PARAM_GERCON>...........................................................// Op1 = SET_PARAM_GERCON
// Op2 = see below
#define SET_PARAM_GERCON_CLEAR_TRIGGER                      (unsigned char)0x01
// Answer from uC: 	Op1 = SET_PARAM_GERCON
//                  Op2 = SET_PARAM_GERCON_CLEAR_TRIGGER
//                  Op3 = State of Gercon (0 or 1), read with antibounce
//                  Op4 = State of Gercon Trigger (1 - gercon was turned on)
//                  Op5 = LAST_ERROR
// <SET_PARAM_OPT_DIAG>.........................................................// Op1 = SET_PARAM_OPT_DIAG
// Op2 = see below
#define SET_PARAM_OPT_DIAG_CLEAR_TRIGGER                    (unsigned char)0x01
// Answer from uC: 	Op1 = SET_PARAM_OPT_DIAG
//                  Op2 = SET_PARAM_OPT_DIAG_CLEAR_TRIGGER
//                  Op3 = State of OptDiag (0 or 1), read with antibounce
//                  Op4 = State of OptDiag Trigger (1 - OptDiag was turned on)
//                  Op5 = LAST_ERROR
// <SET_PARAM_PBTN>.............................................................// Op1 = SET_PARAM_PBTN
// Op2 = see below
#define SET_PARAM_PBTN_CLEAR_TRIGGER                        (unsigned char)0x01
// Answer from uC: 	Op1 = SET_PARAM_PBTN
//                  Op2 = SET_PARAM_PBTN_CLEAR_TRIGGER
//                  Op3 = State of Push Button (0 or 1), read with antibounce
//                  Op4 = State of Push Button Trigger (1 - OptDiag was turned on)
//                  Op5 = LAST_ERROR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define CMD_GET_PARAM                                       (unsigned char)0x87
//	Op1 =
#define GET_PARAM_DEVICE                                    (unsigned char)0x00 // <-- Op1
#define GET_PARAM_ENGINES                                   (unsigned char)0xB2 // <-- Op1
#define GET_PARAM_VOLTAGES                                  (unsigned char)0xB3 // <-- Op1
#define GET_PARAM_BTNS_STATE                                (unsigned char)0xB5 // <-- Op1
#define GET_PARAM_SNS                                       (unsigned char)0xB6 // <-- Op1
#define GET_PARAM_GERCON                                    (unsigned char)0xB7 // <-- Op1
#define GET_PARAM_OPT_DIAG                                  (unsigned char)0xB8 // <-- Op1
#define GET_PARAM_PBTN                                      (unsigned char)0xB9 // <-- Op1

//	<GET_PARAM_DEVICE>..........................................................
#define GET_PARAM_DEVICE_GET_ID                             (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_DEVICE
//                  Op2 = GET_PARAM_DEVICE_GET_ID
//                  Op3 = MSB DEVICE_ID BYTE
//                  Op4 = MIDB1 DEVICE_ID BYTE
//                  Op5 = MIDB0 DEVICE_ID BYTE
//                  Op6 = LSB DEVICE_ID BYTE
//                  Op7 = LAST_ERROR
#define GET_PARAM_DEVICE_GET_FIRMWARE_REVISION	(unsigned char)0x02             // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_DEVICE
//                  Op2 = GET_PARAM_DEVICE_GET_FIRMWARE_REVISION
//                  Op3 = FIRMWARE_REVISION
//                  Op4 = LAST_ERROR
// <GET_PARAM_ENGINES>..........................................................
// Op2 = ENGINE_NUMBER                                                          // Op2 = ENGINE_NUMBER
// Op3 = see below PCODE                                                        // Op3 = PCODE
#define GET_PARAM_ENGINES_PCODE_MAX_SPEED                   (unsigned char)0x00
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_MAX_SPEED
//                  Op4:Op7 = Max Speed Index (Int32)
//                  Op8 = ENGINE_STATUS
//                  Op9 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_REPEATABILITY               (unsigned char)0x03 
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_REPEATABILITY
//                  Op4 = Repeatability (UInt8)
//                  Op5 = ENGINE_STATUS
//                  Op6 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_MICROSTEP_MODE              (unsigned char)0x04 
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_MICROSTEP_MODE
//                  Op4 = Op4 = Mode (UInt8)
//                  Op5 = ENGINE_STATUS
//                  Op6 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_HOME_POSITION               (unsigned char)0x05
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_HOME_POSITION
//                  Op4:Op11 = Position (Int64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_F0                          (unsigned char)0x06
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_F0
//                  Op4:Op11 = F0 (Double64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_FMAX                        (unsigned char)0x07
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_FMAX
//                  Op4:Op11 = FMAX (Double64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_DFMAX                       (unsigned char)0x08
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_DFMAX
//                  Op4:Op11 = DFMAX (Double64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR
#define GET_PARAM_ENGINES_PCODE_GET_POSITION                (unsigned char)0x1C
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_GET_POSITION
//                  Op4:Op11 = Position (Int64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR 
#define GET_PARAM_ENGINES_PCODE_GET_PULSE_COUNTER            (unsigned char)0x1D
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_GET_PULSE_COUNTER
//                  Op4:Op11 = Pulse Counter (Int64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR 
#define GET_PARAM_ENGINES_PCODE_GET_POSITION_AFTER_CROSS    (unsigned char)0x20
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_GET_POSITION_AFTER_CROSS
//                  Op4:Op11 = Position (Int64)
//                  Op12 = ENGINE_STATUS
//                  Op13 = LAST_ERROR 
#define GET_PARAM_ENGINES_PCODE_DRIVER_READ_BUFFER          (unsigned char)0x30
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_PCODE_DRIVER_READ_BUFFER
//                  Op4:Op7 = TMC2660C read data from SPI
//                  Op8 = ENGINE_STATUS
//                  Op9 = LAST_ERROR 
#define GET_PARAM_ENGINES_MIXMODE_STATE                     (unsigned char)0x21
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER WITH MIX MODE
//                  Op3 = GET_PARAM_ENGINES_MIXMOD_STATE
//                  Op4 = Current State (FSM) + MSB bit is time mode On/Off
//                  Op5:Op8 = Current Cycle
//                  Op9:Op12 = Current Time
//                  Op13 = ENGINE_STATUS
//                  Op14 = LAST_ERROR
#define GET_PARAM_ENGINES_OPCODE_ACCEL_AND_STANDBY_CURRENTS (unsigned char)0x22
// 	Answer from uC: Op1 = GET_PARAM_ENGINES
//                  Op2 = ENGINE_NUMBER
//                  Op3 = GET_PARAM_ENGINES_OPCODE_ACCEL_AND_STANDBY_CURRENTS
//                  Op4 = Acceleartion Current
//                  Op5 = Standby Current
//                  Op6 = ENGINE_STATUS
//                  Op7 = LAST_ERROR 
//<GET_PARAM_VOLTAGES>..........................................................
#define GET_PARAM_VOLTAGES_24VMON                           (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_VOLTAGES
//                  Op2 = GET_PARAM_VOLTAGES_24VMON
//                  Op3:Op10 = Voltage (Double64)
//                  Op11 = LAST_ERROR
#define GET_PARAM_VOLTAGES_UVLAMP                           (unsigned char)0x02 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_VOLTAGES
//                  Op2 = GET_PARAM_VOLTAGES_UVLAMP
//                  Op3:Op10 = Voltage (Double64)
//                  Op11 = LAST_ERROR
#define GET_PARAM_VOLTAGES_R2R_DAC                          (unsigned char)0x03 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_VOLTAGES
//                  Op2 = GET_PARAM_VOLTAGES_R2R_DAC
//                  Op3:Op10 = Voltage (Double64)
//                  Op11 = LAST_ERROR
//<GET_PARAM_BTNS_STATE>........................................................
#define GET_PARAM_BTNS_STATE_ALL                            (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_BTNS_STATE
//                  Op2 = GET_PARAM_BTNS_STATE_ALL
//                  Op3 = State of PUSH Button (0 or 1), read from IO port without antibounce
//                  Op4 = State of Gercon (0 or 1), read with antiboounce
//                  Op5 = LAST_ERROR
//<GET_PARAM_SNS>...............................................................
#define GET_PARAM_SNS_STATE_ALL                             (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_SNS
//                  Op2 = GET_PARAM_SNS_STATE_ALL
//                  Op3 = SNS State (1 - object present, 0 - empty)
//                  Op4 = LAST_ERROR
//<GET_PARAM_GERCON>............................................................
#define GET_PARAM_GERCON_STATE                              (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_GERCON
//                  Op2 = GET_PARAM_GERCON_STATE
//                  Op3 = State of Gercon (0 or 1), read with antiboounce
//                  Op4 = State of Gercon Trigger (1 - gercon was turned on)
//                  Op5 = LAST_ERROR
//<GET_PARAM_OPT_DIAG>..........................................................
#define GET_PARAM_OPT_DIAG_STATE                            (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_OPT_DIAG
//                  Op2 = GET_PARAM_OPT_DIAG_STATE
//                  Op3 = State of OPT_DIAG (0 or 1), read with antiboounce
//                  Op4 = State of OPT_DIAG Trigger (1 - optdiag was turned on)
//                  Op5 = LAST_ERROR
//<GET_PARAM_PBTN>..........................................................
#define GET_PARAM_PBTN_STATE                                (unsigned char)0x01 // <-- Op2, Op3,... absent
// 	Answer from uC: Op1 = GET_PARAM_PBTN
//                  Op2 = GET_PARAM_PBTN_STATE
//                  Op3 = State of PUSH Button (0 or 1), read with antiboounce
//                  Op4 = State of PUSH Button Trigger (1 - push button was pressed)
//                  Op5 = LAST_ERROR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //__SRVINTCMD_H__
