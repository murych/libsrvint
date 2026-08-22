/*!
 *
 */

#ifndef SRVINT_PRIVATE_H
#define SRVINT_PRIVATE_H

#include <bits/types/struct_timeval.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>

#include "srvint.h"

SRVINT_BEGIN_DECLS

#define _RESPONSE_TIMEOUT 50000
#define SRVINT_HEADER_LENGTH 6
#define SRVINT_MAX_PAYLOAD 255

typedef struct _srvint_frame {
  uint8_t address;
  uint8_t packet_id;
  uint8_t command;
  uint8_t payload_length;
  uint8_t payload[SRVINT_MAX_PAYLOAD];
} srvint_frame_t;

typedef enum {
  MSG_INDICATION,   // request message on server side
  MSG_CONFIRMATIOM  // request message on client side
} msg_type_t;

#if 0
typedef struct _sft {
  int slave;
  int function;
  int t_id;
} stf_t;

typedef struct _srvint_backend {
  unsigned int backend_type;
  unsigned int header_length;
  unsigned int checksum_length;

  int (*set_slave)(srvint_t* ctx, int slave);
  int (*send)(srvint_t* ctx, uint8_t* msg, int msg_length);
  void (*free)(srvint_t* ctx);
} srvint_backend_t;
#endif

typedef struct _srvint_serial {
  char* device;
  int baud;
  uint8_t data_bits;
  uint8_t stop_bits;
  char parity;
  struct termios old_tios;
  int rts;
  int rts_delay;
  int confirmation_to_ignore;
} srvint_serial_t;

struct _srvint {
  int slave;
  int s;
  int debug;
  int error_recovery;
  struct timeval response_timeout;
#if 0
  const srvint_backend_t* backend;
#endif
  srvint_serial_t* backend_data;
  uint8_t last_packet_id;
};

struct _srvint_server {
  srvint_t *transport;
  int stop_requested;
  int running;
};

void _srvint_init_common(srvint_t* ctx);
void _error_print(srvint_t* ctx, const char* context);
int _srvint_recieve_msg(srvint_t* ctx, uint8_t* msg, msg_type_t msg_type);

uint8_t _srvint_compute_hdr_hash(uint8_t addr, uint8_t pid, uint8_t cmd,
                                 uint8_t length);
uint8_t _srvint_compute_payload_hash(const uint8_t* data, uint8_t length);
uint8_t _srvint_next_packet_id(srvint_t* ctx);

int _srvint_send(srvint_t* ctx, const uint8_t* frame, size_t length);

SRVINT_END_DECLS

#endif  // SRVINT_PRIVATE_H
