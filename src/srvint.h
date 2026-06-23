#ifndef SRVINT_H
#define SRVINT_H

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

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

/* srvint function codes */

#define SRVINT_FC_GO_TO_BOOT_MODE 0x77
#define SRVINT_FC_PING            0x81
#define SRVINT_FC_SW_RESET        0x82
#define SRVINT_FC_HW_RESET        0x83
#define SRVINT_FC_GET_ERROR       0x84
#define SRVINT_FC_ZEROIZE_ERROR   0x85
#define SRVINT_FC_SET_PARAM       0x86
#define SRVINT_FC_GET_PARAM       0x87

#define SRVINT_DEVICE_ADDRESS    0x10
#define SRVINT_NULL_ADDRESS      0x00
#define SRVINT_BROADCAST_ADDRESS 0x55
#define SRVINT_MASTER_ADDRESS    0xCA

// errors

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

#define ESIXUNKOPS (SRVINT_ENOBASE + SRVINT_UNKNOWN_OPERANDS_VALUE)

/* native libsvint error codes */

#define ESIBADCRC  (ESIXUNKOPS + 1)
#define ESIBADDATA (ESIXUNKOPS + 2)

extern const unsigned int libsrvint_version_major;
extern const unsigned int libsrvint_version_minor;
extern const unsigned int libsrvint_version_patch;

typedef struct _srvint srvint_t;

typedef enum {
  SRVINT_ERROR_RECOVERY_NONE = 0,
  SRVINT_ERROR_RECOVERY_LINK = (1 << 1),
  SRVINT_ERROR_RECOVERY_PROTOCOL = (1 << 2)
} srvint_error_recovery_mode;

SRVINT_API int srvint_set_slave(srvint_t* ctx, int slave);
SRVINT_API int srvint_get_slave(srvint_t* ctx);

SRVINT_API int srvint_set_reponse_timeout(srvint_t* ctx, uint32_t to_sec,
                                          uint32_t to_usec);
SRVINT_API int srvint_get_response_timeout(srvint_t* ctx, uint32_t* to_sec,
                                           uint32_t* to_usec);

SRVINT_API int srvint_get_header_length(srvint_t* ctx);

SRVINT_API int srvint_connect(srvint_t* ctx);
SRVINT_API int srvint_close(srvint_t* ctx);

SRVINT_API void srvint_free(srvint_t* ctx);

SRVINT_API int srvint_flush(srvint_t* ctx);
SRVINT_API int srvint_set_debug(srvint_t* ctx, int flag);

SRVINT_API const char* srvint_strerror(int errnum);
SRVINT_API srvint_t* srvint_serial_new(const char* device, int baud,
                                       char parity, int data_bit, int stop_bit);
SRVINT_END_DECLS

#endif  // SRVINT_H
