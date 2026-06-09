#ifndef __ERRORS_H__
#define __ERRORS_H__
/* 
 * File:   Errors.h
 * Author: Maxim Galkin
 * Rev. 1.1
 */
//................................................................................................
#define NO_ERRORS                                                               (unsigned int)0
#define INCORRECT_ERROR_POSITION                                                0xFFFF0001
//................................................................................................
/* SPI.c */
#define ERROR_SPI_INCORRECT_SPI_NUMBER                                          0x00010099
//................................................................................................
// UART.c
#define ERROR_UART_INCORRECT_UART_NUMBER                                        0x00070021
#define ERROR_UART1_FERR_OCCURED                                                0x00070022
#define ERROR_UART2_FERR_OCCURED                                                0x00070023
#define ERROR_UART3_FERR_OCCURED                                                0x00070024
#define ERROR_UART4_FERR_OCCURED                                                0x00070025
#define ERROR_UART5_FERR_OCCURED                                                0x00070026
#define ERROR_UART6_FERR_OCCURED                                                0x00070027

#define ERROR_UART1_OVERRUN_OCCURED                                             0x00070028
#define ERROR_UART2_OVERRUN_OCCURED                                             0x00070029
#define ERROR_UART3_OVERRUN_OCCURED                                             0x0007002A
#define ERROR_UART4_OVERRUN_OCCURED                                             0x0007002B
#define ERROR_UART5_OVERRUN_OCCURED                                             0x0007002C
#define ERROR_UART6_OVERRUN_OCCURED                                             0x0007002D

#define ERROR_UART1_INPUT_BUFFER_OVERFLOW                                       0x0007002E
#define ERROR_UART2_INPUT_BUFFER_OVERFLOW                                       0x0007002F
#define ERROR_UART3_INPUT_BUFFER_OVERFLOW                                       0x00070030
#define ERROR_UART4_INPUT_BUFFER_OVERFLOW                                       0x00070031
#define ERROR_UART5_INPUT_BUFFER_OVERFLOW                                       0x00070032
#define ERROR_UART6_INPUT_BUFFER_OVERFLOW                                       0x00070033
//................................................................................................
// SMMs.c
#define ERROR_SMMS_INCORRECT_SMM_NUMBER                                         0x000800A1
#define ERROR_SMMS_INCORRECT_DATA_FOR_WRITE_TO_TMC2660                          0x000800A2
#define ERROR_SMMS_CANNOT_WRITE_INTO_TMC2660_WHEN_WRITING_NOT_COMPLETE          0x000800A3
#define ERROR_SMMS_INCORRECT_ENGINE_NUMBER                                      0x000800A4
#define ERROR_ENGINE_NOT_FREE_TIMERS_NOW                                        0x000800B0
                                                                                // B0.....BF
#define ERROR_UNDEFINED_TMR5_ISR                                                0x000800A6
#define ERROR_UNDEFINED_TMR7_ISR                                                0x000800A7
#define ERROR_UNDEFINED_TMR9_ISR                                                0x000800A8
/*
#define ERROR_SMMS_SET_DATA_LARGE_CAL_MAX_SPEED                                 0x000800B1
#define ERROR_SMMS_REPEATABILITY_CAN_NOT_BE_ZERO                                0x000800B2
#define ERROR_SMMS_REPEATABILITY_CAN_NOT_BE_MORE_THEN_250                       0x000800B3
#define ERROR_SMMS_WINDING_CURRENT_CAN_NOT_BE_MORE_THEN_1023                    0x000800B4
#define ERROR_SMMS_ACCELERATION_WINDING_CURRENT_CAN_NOT_BE_MORE_THEN_1023       0x000800B5
#define ERROR_SMMS_UNKNOWN_PARAMETER_NUMBER                                     0x000800B6
*/
#define ERROR_ENGINE_CAN_NOT_FIND_ZERO_SWITCH_1                                 0x00080001
#define ERROR_ENGINE_CAN_NOT_FIND_ZERO_SWITCH_2                                 0x00080003
#define ERROR_ENGINE_CAN_NOT_MOVE_WHEN_UNDEFINED_POSITION                       0x00080005
#define ERROR_ENGINE_CAN_NOT_MOVE_WHEN_MOVING_TO_ZERO                           0x00080006
#define ERROR_ENGINE_CROSS_UNDEFINED_SWITCH                                     0x00080007
#define ERROR_ENGINE_CROSS_SWITCH1                                              0x00080008
#define ERROR_ENGINE_CROSS_SWITCH2                                              0x00080009
#define ERROR_DIFFERENCE_BETWEEN_ABSOLUTE_POSITION_AND_STEPS_IS_NEGATIVE        0x0008000D
#define ERROR_SUM_ABSOLUTE_POSITION_AND_STEPS_IS_VERY_BIG                       0x0008000E
#define ERROR_TMR5_OVERFLOW                                                     0x0008000F
#define ERROR_TMR7_OVERFLOW                                                     0x00080010
#define ERROR_TMR9_OVERFLOW                                                     0x00080011
#define ERROR_ENGINE_CAN_NOT_GOTO_N_STEP_WHEN_MOVING                            0x00080012
//................................................................................................
// SpecFunc.c
#define ERROR_SFUNC_CAN_NOT_START_WHEN_WORKING_NOW                              0x00020041
#define ERROR_SFUNC_CAN_NOT_START_FROM_UNDEFINED_POSITION                       0x00020042
#define ERROR_SFUNC_CAN_NOT_START_WHEN_MOVING_TO_ZERO                           0x00020043
//................................................................................................
// SrvInt.c
#define ERROR_SRVINT_RECEIVED_UNKNOWN_CMD                                       0x00030050
#define ERROR_SRVINT_EXCEEDED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_CONTROL_BYTES 0x00030051
#define ERROR_SRVINT_RECEIVED_HASH_NOT_EQUAL_CALCULATING_HASH                   0x00030052
#define ERROR_SRVINT_CAN_NOT_RECIEVE_OPERANDS_MORE_THEN_BUFFER_CAPACITY         0x00030053
#define ERROR_SRVINT_EXCEEDED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_OPERANDS_BYTES 0x00030054
#define ERROR_SRVINT_RECEIVED_DATA_HASH_NOT_EQUAL_CALCULATING_HASH              0x00030055
#define ERROR_SRVINT_RECEIVED_PACKET_WITH_ZERO_MSB_BIT_IN_PACKET_ID             0x00030056
#define ERROR_SRVINT_SAME_CMD_WITH_SAME_PACKETID_CAN_NOT_GO_SUCCESSIVELY        0x00030057
#define ERROR_SRVINT_UART_INPUT_BUFFER_OVERFLOW                                 0x00030058
#define ERROR_SRVINT_UNKNOWN_OPERANDS_VALUE                                     0x00030059
//...............................................................................................
// FRAM.c
#define ERROR_FRAM_WRITE_INTO_STATUS_REGISTER_FAILED                            0x000D0061
//...............................................................................................
// SrvIntM.c
#define ERROR_SRVINTM_INCORRECT_OPERAND_INDEX                                   0x000E0091
#define ERROR_SRVINTM_UART_INPUT_BUFFER_OVERFLOW                                0x000E0092
#define ERROR_SRVINTM_EXCEED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_CONTROL_BYTES  0x000E0093  
#define ERROR_SRVINTM_RECEIVED_HASH_NOT_EQUAL_CALCULATING_HASH                  0x000E0094
#define ERROR_SRVINTM_CAN_NOT_RECIEVE_OPERANDS_MORE_THEN_BUFFER_CAPACITY        0x000E0095
#define ERROR_SRVINTM_EXCEEDED_MAX_TIME_LATENCY_BETWEEN_PACKET_OPERANDS_BYTES   0x000E0096
#define ERROR_SRVINTM_RECEIVED_DATA_HASH_NOT_EQUAL_CALCULATING_HASH             0x000E0097
#define ERROR_SRVINTM_RECEIVED_PACKET_WITH_ZERO_MSB_BIT_IN_PACKET_ID            0x000E0098
#define ERROR_SRVINTM_SAME_CMD_WITH_SAME_PACKETID_CAN_NOT_GO_SUCCESSIVELY       0x000E0099
#define ERROR_SRVINMT_UNKNOWN_OPERANDS_VALUE                                    0x000E009A
#define ERROR_SRVINTM_RECEIVED_UNKNOWN_CMD                                      0x000E009B
//...............................................................................................
// VMeasure.c
#define ERROR_VM_INCORRECT_ADCFILTER_INDEX                                      0x000A00D0
#define ERROR_VM_INCORRECT_RAIL_INDEX                                           0x000A00D1
//...............................................................................................
#define ERROR_POWER_INCORRECT_FSM_STATE                                         0x000B0070
// Power.c
//...............................................................................................
//main.c
#define ERROR_MAIN_STORE_FAILED                                                 0x000900F0
#define ERROR_STIL_IS_DEAD                                                      0x000900F1
//................................................................................................
#define ERROR_BUFFER_ON
#ifdef ERROR_BUFFER_ON
#define ERROR_BUFFER_SIZE                                                       (unsigned int)200
typedef struct
{
	unsigned int ErrVal;
	unsigned int Time;
    unsigned int ErrLine;
}ErrStructType;

unsigned int Get_Error_Buffer_Pointer(void);
ErrStructType Get_Previous_Error(unsigned int Offset);
ErrStructType Get_Error(unsigned int ErrorPosition);

#define LAST_ERROR			(unsigned int)0
#endif

#ifndef ERROR_BUFFER_ON
unsigned int Get_Error(void);
#endif

void Zeroize_Error(void);
void Clear_Last_Error(void);
unsigned int Get_Errors_Count(void);
void Set_Error(unsigned int line,unsigned int ErrVal);

#endif //__ERRORS_H__