/**
 * @file srvint.h
 * @brief Public C99 API for SrvInt serial communication.
 *
 * The API supports both client request/response operations and application-
 * driven server-style receive/reply processing. Parameter values remain raw
 * byte sequences and are interpreted by the application.
 */

#ifndef SRVINT_H
#define SRVINT_H

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <stddef.h>
#include <stdint.h>

#include "libsrvint_export.h"

#ifdef __cplusplus
#define SRVINT_BEGIN_DECLS extern "C" {
#define SRVINT_END_DECLS   }
#else
#define SRVINT_BEGIN_DECLS
#define SRVINT_END_DECLS
#endif  // __cplusplus

SRVINT_BEGIN_DECLS

/** @brief Start byte used by SrvInt frames. */
/* Override with -DSRVINT_START_BYTE=0xb7 when building for another variant. */
#ifndef SRVINT_START_BYTE
#define SRVINT_START_BYTE 0x55u
#endif

#ifndef FALSE
#define FALSE 0
#endif  // FALSE

#ifndef TRUE
#define TRUE 1
#endif  // TRUE

#ifndef OFF
#define OFF 0
#endif  // OFF

#ifndef ON
#define ON 1
#endif  // ON

/** @name SrvInt function codes
 *  @{ */

#define SRVINT_FC_GO_TO_BOOT_MODE 0x77
#define SRVINT_FC_UNKNOWN         0x00
#define SRVINT_FC_PING            0x81
#define SRVINT_FC_HW_RESET        0x82
#define SRVINT_FC_SW_RESET        0x83
#define SRVINT_FC_GET_ERROR       0x84
#define SRVINT_FC_ZEROIZE_ERROR   0x85
#define SRVINT_FC_SET_PARAM       0x86
#define SRVINT_FC_GET_PARAM       0x87

/** @} */

/** @name SrvInt addresses
 *  @{ */
#define SRVINT_DEVICE_ADDRESS    0x10
#define SRVINT_NULL_ADDRESS      0x00
#define SRVINT_BROADCAST_ADDRESS 0x55
#define SRVINT_MASTER_ADDRESS    0xCA

/** @} */

/** @name SrvInt error codes
 *  The low byte is the protocol error value; public error macros add the
 *  SrvInt error namespace base.
 *  @{ */

#define SRVINT_ENOBASE (uint32_t)0x00030000

enum {
  SRVINT_ERROR_RECIEVED_UNKNOWN_CMD = 0x50,
  SRVINT_EXCEEDED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_CONTROL_BYTES,
  SRVINT_RECIEVED_HASH_NOT_EQUAL_CALCULATING_HASH,
  SRVINT_CAN_NOT_RECIEVE_OPERANDS_MORE_THAN_BUFFER_CAPACITY,
  SRVINT_EXCEEDED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_OPERANDS_BYTES,
  SRVINT_RECEIVED_DATA_HASH_NOT_EQUAL_CALCULATING_HASH,
  SRVINT_RECEIVED_PACKET_WITH_ZERO_MSB_BIT_IN_PACKET_ID,
  SRVINT_SAME_CMD_WITH_SAME_PACKETID_CAN_NOT_GO_SUCCESSIVELY,
  SRVINT_UART_INPUT_BUFFER_OVERFLOW,
  SRVINT_UNKNOWN_OPERANDS_VALUE,
};

#define ESIXUNCM (SRVINT_ENOBASE + SRVINT_ERROR_RECIEVED_UNKNOWN_CMD)
#define ESIXMAXLAT  \
  (SRVINT_ENOBASE + \
   SRVINT_EXCEEDED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_CONTROL_BYTES)
#define ESIXRECHASH \
  (SRVINT_ENOBASE + SRVINT_RECIEVED_HASH_NOT_EQUAL_CALCULATING_HASH)
#define ESIXOPSCAP \
  (SRVINT_ENOBASE + SRVINT_CAN_NOT_RECIEVE_OPERANDS_MORE_THAN_BUFFER_CAPACITY)
#define ESIXMAXOPLAT \
  (SRVINT_ENOBASE +  \
   SRVINT_EXCEEDED_MAXIMUM_TIME_LATENCY_BETWEEN_PACKET_OPERANDS_BYTES)
#define ESIXRECDHASH \
  (SRVINT_ENOBASE + SRVINT_RECEIVED_DATA_HASH_NOT_EQUAL_CALCULATING_HASH)
#define ESIXPIDMSB \
  (SRVINT_ENOBASE + SRVINT_RECEIVED_PACKET_WITH_ZERO_MSB_BIT_IN_PACKET_ID)
#define ESIXSAMEPID \
  (SRVINT_ENOBASE + SRVINT_SAME_CMD_WITH_SAME_PACKETID_CAN_NOT_GO_SUCCESSIVELY)
#define ESIXUARTOVF (SRVINT_ENOBASE + SRVINT_UART_INPUT_BUFFER_OVERFLOW)
#define ESIXUNKOPS  (SRVINT_ENOBASE + SRVINT_UNKNOWN_OPERANDS_VALUE)

/* native libsvint error codes */

#define ESIBADCRC  (ESIXUNKOPS + 1)
#define ESIBADDATA (ESIXUNKOPS + 2)

/** @} */

extern const unsigned int libsrvint_version_major;
extern const unsigned int libsrvint_version_minor;
extern const unsigned int libsrvint_version_patch;

/** @brief Opaque serial transport and protocol context. */
typedef struct _srvint srvint_t;

/**
 * @brief Application callback used to handle raw GET_PARAM and SET_PARAM
 *        requests.
 *
 * The request is a complete validated wire frame. The callback owns the
 * interpretation of its operands and writes only the response payload.
 *
 * @param ctx Active SrvInt context.
 * @param request Complete validated request frame.
 * @param request_length Request frame length in bytes.
 * @param response Response payload destination.
 * @param response_capacity Response payload capacity.
 * @param response_length Receives the response payload length.
 * @param user_data Application-defined callback data.
 * @return 0 on success, or a negative value on callback error.
 */
typedef int (*srvint_param_callback_t)(srvint_t* ctx, const uint8_t* request,
                                       size_t request_length, uint8_t* response,
                                       size_t response_capacity,
                                       size_t* response_length,
                                       void* user_data);

/** @brief Error recovery modes used by the serial transport. */
typedef enum {
  SRVINT_ERROR_RECOVERY_NONE = 0,
  SRVINT_ERROR_RECOVERY_LINK = (1 << 1),
  SRVINT_ERROR_RECOVERY_PROTOCOL = (1 << 2)
} srvint_error_recovery_mode;

/** @brief Set the local slave address. */
SRVINT_API int srvint_set_slave(srvint_t* ctx, int slave);

/** @brief Return the configured local slave address. */
SRVINT_API int srvint_get_slave(srvint_t* ctx);

/**
 * @brief Set the response timeout.
 * @param ctx Transport context.
 * @param to_sec Timeout seconds.
 * @param to_usec Additional timeout microseconds.
 * @return 0 on success, or -1 on error.
 */
SRVINT_API int srvint_set_response_timeout(srvint_t* ctx, uint32_t to_sec,
                                           uint32_t to_usec);

/** @brief Compatibility spelling of srvint_set_response_timeout(). */
SRVINT_API int srvint_set_reponse_timeout(srvint_t* ctx, uint32_t to_sec,
                                          uint32_t to_usec);

/**
 * @brief Get the response timeout.
 * @param ctx Transport context.
 * @param to_sec Receives timeout seconds.
 * @param to_usec Receives timeout microseconds.
 * @return 0 on success, or -1 on error.
 */
SRVINT_API int srvint_get_response_timeout(srvint_t* ctx, uint32_t* to_sec,
                                           uint32_t* to_usec);

/** @brief Return the protocol header length in bytes. */
SRVINT_API int srvint_get_header_length(srvint_t* ctx);

/** @brief Open the configured serial device. */
SRVINT_API int srvint_connect(srvint_t* ctx);

/** @brief Close the serial device. */
SRVINT_API int srvint_close(srvint_t* ctx);

/** @brief Release a context. Accepts NULL. */
SRVINT_API void srvint_free(srvint_t* ctx);

/** @brief Flush pending serial input and output. */
SRVINT_API int srvint_flush(srvint_t* ctx);

/**
 * @brief Enable or disable diagnostic RX/TX logging to stdout.
 * @param ctx Transport context.
 * @param flag TRUE to enable logging, FALSE to disable it.
 */
SRVINT_API int srvint_set_debug(srvint_t* ctx, int flag);

/** @brief Convert a library or SrvInt error number to a text description. */
SRVINT_API const char* srvint_strerror(int errnum);

/**
 * @brief Create a serial SrvInt context.
 * @param device Serial device path.
 * @param baud Baud rate.
 * @param parity Parity character: 'N', 'E', or 'O'.
 * @param data_bit Number of data bits.
 * @param stop_bit Number of stop bits.
 * @return New context, or NULL on error.
 */
SRVINT_API srvint_t* srvint_serial_new(const char* device, int baud,
                                       char parity, int data_bit, int stop_bit);

/**
 * @brief Send a raw request and receive its raw response.
 * @param ctx Transport context.
 * @param function SrvInt function code.
 * @param request Request operands, or NULL when request_length is zero.
 * @param request_length Operand length in bytes.
 * @param response Destination response buffer.
 * @param response_capacity Response buffer capacity.
 * @return Response length, or -1 on error.
 */
SRVINT_API int srvint_request(srvint_t* ctx, uint8_t function,
                              const uint8_t* request, uint8_t request_length,
                              uint8_t* response, uint8_t response_capacity);

/** @brief Send PING and receive the remote last-error value. */
SRVINT_API int srvint_ping(srvint_t* ctx, uint8_t* last_error);

/** @brief Send hardware reset and receive the remote last-error value. */
SRVINT_API int srvint_hw_reset(srvint_t* ctx, uint8_t* last_error);

/** @brief Send software reset and receive the remote last-error value. */
SRVINT_API int srvint_sw_reset(srvint_t* ctx, uint8_t* last_error);

/**
 * @brief Read one remote error entry.
 * @param error_position Error entry index.
 * @param error_value Receives the entry value.
 * @param last_error Receives the remote last-error value.
 */
SRVINT_API int srvint_get_error(srvint_t* ctx, uint8_t error_position,
                                uint8_t* error_value, uint8_t* last_error);

/** @brief Clear remote errors and receive the resulting last-error value. */
SRVINT_API int srvint_zeroize_error(srvint_t* ctx, uint8_t* last_error);

/**
 * @brief Send an application-defined command and expect CMD_UNKNOWN.
 * @param command Command code other than SRVINT_FC_UNKNOWN.
 * @param operands Raw command operands.
 * @param operand_count Number of operand bytes.
 * @param last_error Receives the response last-error value.
 */
SRVINT_API int srvint_unknown(srvint_t* ctx, uint8_t command,
                              const uint8_t* operands, uint8_t operand_count,
                              uint8_t* last_error);

/** @brief Send raw SET_PARAM operands and return a raw response. */
SRVINT_API int srvint_set_param(srvint_t* ctx, const uint8_t* request,
                                uint8_t request_length, uint8_t* response,
                                uint8_t response_capacity);

/** @brief Send raw GET_PARAM operands and return a raw response. */
SRVINT_API int srvint_get_param(srvint_t* ctx, const uint8_t* request,
                                uint8_t request_length, uint8_t* response,
                                uint8_t response_capacity);

/**
 * @brief Receive one validated request frame for server-style processing.
 * @param ctx Connected slave context.
 * @param request Destination complete wire-frame buffer.
 * @param request_capacity Destination buffer capacity.
 * @return Complete frame length, or -1 on timeout, malformed frame, or I/O
 *         error.
 */
SRVINT_API int srvint_receive(srvint_t* ctx, uint8_t* request,
                              size_t request_capacity);

/**
 * @brief Form and send a response to a frame returned by srvint_receive().
 *
 * Built-in commands are handled by the library. The callback is used for
 * GET_PARAM and SET_PARAM and receives the complete validated request frame.
 * Broadcast requests may invoke the callback but never produce a response.
 */
SRVINT_API int srvint_reply(srvint_t* ctx, const uint8_t* request,
                            size_t request_length,
                            srvint_param_callback_t callback, void* user_data);

/** @brief Set the protocol-level last-error value used in server replies. */
SRVINT_API int srvint_set_last_error(srvint_t* ctx, uint8_t last_error);

/** @brief Set one protocol-level error entry. */
SRVINT_API int srvint_set_error(srvint_t* ctx, uint8_t error_position,
                                uint8_t error_value);

/** @brief Clear all protocol-level server errors. */
SRVINT_API int srvint_zeroize_errors(srvint_t* ctx);

SRVINT_END_DECLS

#endif  // SRVINT_H
