
#include "srvint.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "srvint_private.h"

#define START_BYTE ((uint8_t)SRVINT_START_BYTE)

const unsigned int libsrvint_version_major = 0;
const unsigned int libsrvint_version_minor = 1;
const unsigned int libsrvint_version_patch = 0;

const char *srvint_strerror(int errnum) {
  switch (errnum) {
    case ESIXUNCM:
      return "Recieved unknown cmd code";
    case ESIXMAXLAT:
      return "Exceeded maximum time latency between packet control bytes";
    case ESIXRECHASH:
      return "Recieved hash not equal calculating hash";
    default:
      return strerror(errnum);
  }
}

void _error_print(srvint_t *ctx, const char *context) {
  if (ctx->debug) {
    fprintf(stderr, "ERROR: %s", srvint_strerror(errno));
    if (context != NULL) {
      fprintf(stderr, ": %s\n", context);
    } else {
      fprintf(stderr, "\n");
    }
  }
}

void srvint_free(srvint_t *ctx) {
  if (ctx == NULL) {
    return;
  }

  if (ctx->backend_data != NULL) {
    free(((srvint_serial_t *)ctx->backend_data)->device);
    free(ctx->backend_data);
  }
  free(ctx);
}

static void _sleep_response_timeout(srvint_t *ctx) {
  struct timespec request, remaining;
  request.tv_sec = ctx->response_timeout.tv_sec;
  request.tv_nsec = ((long int)ctx->response_timeout.tv_usec) * 1000;
  while (nanosleep(&request, &remaining) == -EXIT_FAILURE && errno == EINTR) {
    request = remaining;
  }
}

static int send_msg(srvint_t *ctx, uint8_t *msg, int msg_length) {
  int rc = 0;
  int i = 0;

  // msg_length = ctx->backend->send_msg_pre(msg, msg_length);

  if (ctx->debug) {
    for (i = 0; i < msg_length; i++) {
      printf("[%.2X]", msg[i]);
    }
    printf("\n");
  }

  do {
    rc = _srvint_send(ctx, msg, msg_length);  // TODO
    if (rc == -EXIT_FAILURE) {
      _error_print(ctx, NULL);
      if (ctx->error_recovery & SRVINT_ERROR_RECOVERY_LINK) {
        int saved_errno = errno;
        switch (errno) {
          case EBADF:
          case ECONNRESET:
          case EPIPE: {
            srvint_close(ctx);
            _sleep_response_timeout(ctx);
            srvint_connect(ctx);
            break;
          }
          default: {
            _sleep_response_timeout(ctx);
            srvint_flush(ctx);
            break;
          }
        }
        errno = saved_errno;
      }
    }

  } while ((ctx->error_recovery & SRVINT_ERROR_RECOVERY_LINK) &&
           rc == -EXIT_FAILURE);

  if (rc > EXIT_SUCCESS && rc != msg_length) {
    errno = ESIBADDATA;
    return -1;
  }

  return rc;
}

int _srvint_send(srvint_t *ctx, const uint8_t *frame, size_t length) {
  if (ctx == NULL || ctx->s < 0) {
    errno = EINVAL;
    return -1;
  }

  if (ctx->debug) {
    printf("[SRV TX] ");
    for (size_t i = 0; i < length; i++) {
      printf("%02x ", frame[i]);
    }

  }

  size_t sent = 0;
  while (sent < length) {
    ssize_t n = write(ctx->s, frame + sent, length - sent);
    if (n > 0) { sent += (size_t)n; continue; }
    if (n < 0 && errno == EINTR) continue;
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      struct pollfd pfd = {ctx->s, POLLOUT, 0};
      if (poll(&pfd, 1, -1) < 0 && errno != EINTR) return -1;
      continue;
    }
    if (n == 0) errno = EIO;
    return -1;
  }
  if (tcdrain(ctx->s) < 0) return -1;
  return (int)sent;
}

// ------------------------------------

void _srvint_init_common(srvint_t *ctx) {
  ctx->slave = -1;
  ctx->s = -1;

  ctx->debug = FALSE;
  ctx->error_recovery = SRVINT_ERROR_RECOVERY_NONE;

  ctx->response_timeout.tv_sec = 0;
  ctx->response_timeout.tv_usec = _RESPONSE_TIMEOUT;
}

int srvint_set_slave(srvint_t *ctx, int slave) {
  if (ctx == NULL) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }

  int max_slave = 255;

  if (slave >= SRVINT_NULL_ADDRESS && slave <= max_slave) {
    ctx->slave = slave;
  } else {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int srvint_get_slave(srvint_t *ctx) {
  if (ctx == NULL) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }
  return ctx->slave;
}

int srvint_set_response_timeout(srvint_t *ctx, uint32_t to_sec,
                                uint32_t to_usec) {
  if (ctx == NULL || (to_sec == 0 && to_usec == 0) || to_usec > 999999) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }

  ctx->response_timeout.tv_sec = to_sec;
  ctx->response_timeout.tv_usec = to_usec;
  return EXIT_SUCCESS;
}

int srvint_get_response_timeout(srvint_t *ctx, uint32_t *to_sec,
                                uint32_t *to_usec) {
  if (ctx == NULL || to_sec == NULL || to_usec == NULL) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }

  *to_sec = ctx->response_timeout.tv_sec;
  *to_usec = ctx->response_timeout.tv_usec;
  return EXIT_SUCCESS;
}

int srvint_set_reponse_timeout(srvint_t *ctx, uint32_t to_sec,
                               uint32_t to_usec) {
  return srvint_set_response_timeout(ctx, to_sec, to_usec);
}

static speed_t baud_to_speed(int baud) {
  switch (baud) {
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    default:
      errno = EINVAL;
      return (speed_t)0;
  }
}

int srvint_connect(srvint_t *ctx) {
  if (ctx == NULL) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }

  if (ctx->backend_data == NULL) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }
  if (ctx->s >= 0) return EXIT_SUCCESS;
  srvint_serial_t *ctx_serial = ctx->backend_data;

  if (ctx->debug) {
  }

  int flags = O_RDWR | O_NOCTTY | O_NONBLOCK;
#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif

  ctx->s = open(ctx_serial->device, flags);
  if (ctx->s < EXIT_SUCCESS) {
    if (ctx->debug) {
      fprintf(stderr, "ERROR can't open device %s (%s)\n", ctx_serial->device,
              strerror(errno));
    }
    return -EXIT_FAILURE;
  }

  if (tcgetattr(ctx->s, &ctx_serial->old_tios) != EXIT_SUCCESS) {
    close(ctx->s);
    ctx->s = -1;
    if (ctx->debug) {
      fprintf(stderr, "ERROR can't get system serial options\n");
    }
    return -EXIT_FAILURE;
  }

  struct termios tty;
  if (tcgetattr(ctx->s, &tty) != EXIT_SUCCESS) {
    close(ctx->s);
    ctx->s = -1;
    return -EXIT_FAILURE;
  }

  tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | ISIG | ICANON);

  if (ctx_serial->parity == 'N') {
    tty.c_iflag &= ~INPCK;
  } else {
    tty.c_iflag |= INPCK;
  }

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_oflag &= ~(OPOST | ONLCR);

  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
  tty.c_cflag &= ~(PARENB | CSTOPB);
#ifdef CRTSCTS
  tty.c_cflag &= ~CRTSCTS;
#endif
  tty.c_cflag |= CREAD | CLOCAL;
  tty.c_cflag &= ~CSIZE;
  switch (ctx_serial->data_bits) {
    case 5: tty.c_cflag |= CS5; break;
    case 6: tty.c_cflag |= CS6; break;
    case 7: tty.c_cflag |= CS7; break;
    case 8: tty.c_cflag |= CS8; break;
    default:
      errno = EINVAL;
      close(ctx->s); ctx->s = -1;
      return -EXIT_FAILURE;
  }

  if (ctx_serial->stop_bits == 1) {
    tty.c_cflag &= ~CSTOPB;
  } else {
    tty.c_cflag |= CSTOPB;
  }

  if (ctx_serial->parity == 'N') {
    tty.c_cflag &= ~PARENB;
  } else if (ctx_serial->parity == 'E') {
    tty.c_cflag |= PARENB;
    tty.c_cflag &= ~PARODD;
  } else {
    tty.c_cflag |= PARENB;
    tty.c_cflag |= PARODD;
  }

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  if (cfsetispeed(&tty, ctx_serial->baud) < EXIT_SUCCESS ||
      cfsetospeed(&tty, ctx_serial->baud) < EXIT_SUCCESS) {
    if (ctx->debug) {
      fprintf(stderr, "ERROR can't update tx baudrater\n");
    }
    close(ctx->s);
    ctx->s = -1;
    return -EXIT_FAILURE;
  }

  if (tcsetattr(ctx->s, TCSANOW, &tty) < EXIT_SUCCESS) {
    close(ctx->s);
    ctx->s = -1;
    if (ctx->debug) {
      fprintf(stderr, "ERROR can't update system serial options\n");
    }
    return -EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int srvint_close(srvint_t *ctx) {
  if (ctx != NULL && ctx->s >= 0) {
    tcsetattr(ctx->s, TCSANOW, &ctx->backend_data->old_tios);
    close(ctx->s);
    ctx->s = -1;
  }
  return EXIT_SUCCESS;
}

int srvint_set_debug(srvint_t *ctx, int flag) {
  if (ctx == NULL) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }
  ctx->debug = flag;
  return EXIT_SUCCESS;
}

srvint_t *srvint_serial_new(const char *device, int baud, char parity,
                            int data_bit, int stop_bit) {
#if 0
  srvint_t *ctx = calloc(1, sizeof(srvint_t));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->slave = 1;
  ctx->response_timeout.tv_usec = 1000;
  ctx->debug = FALSE;

  srvint_serial_t *backend = ctx->backend_data;
  strncpy(backend->device, device, sizeof(backend->device) - 1);
  backend->baud = baud != 0 ? baud : 115200;
  backend->data_bits = data_bit != 0 ? data_bit : 8;
  backend->stop_bits = stop_bit != 0 ? stop_bit : 1;

  return ctx;
#else

  srvint_t *ctx;
  srvint_serial_t *serial;

  if (device == NULL || *device == 0) {
    fprintf(stderr, "the device string is empty\n");
    errno = EINVAL;
    return NULL;
  }

  if (baud == 0 || baud_to_speed(baud) == 0) {
    errno = EINVAL;
    return NULL;
  }

  ctx = (srvint_t *)malloc(sizeof(srvint_t));
  if (ctx == NULL) {
    return NULL;
  }

  _srvint_init_common(ctx);

  ctx->backend_data = (srvint_serial_t *)malloc(sizeof(srvint_serial_t));
  if (ctx->backend_data == NULL) {
    srvint_free(ctx);
    errno = ENOMEM;
    return NULL;
  }

  serial = (srvint_serial_t *)ctx->backend_data;
  memset(serial, 0, sizeof(*serial));
  serial->device = (char *)malloc((strlen(device) + 1) * sizeof(char));
  if (serial->device == NULL) {
    srvint_free(ctx);
    errno = ENOMEM;
    return NULL;
  }
  strcpy(serial->device, device);

  serial->baud = baud_to_speed(baud);
  serial->data_bits = data_bit != 0 ? (uint8_t)data_bit : 8;
  serial->stop_bits = stop_bit != 0 ? (uint8_t)stop_bit : 1;
  if (serial->data_bits < 5 || serial->data_bits > 8 ||
      (serial->stop_bits != 1 && serial->stop_bits != 2)) {
    srvint_free(ctx);
    errno = EINVAL;
    return NULL;
  }
  if (parity == 'N' || parity == 'E' || parity == 'O') {
    serial->parity = parity;
  } else {
    srvint_free(ctx);
    errno = EINVAL;
    return NULL;
  }

  return ctx;
#endif
}

uint8_t _srvint_next_packet_id(srvint_t *ctx) {
  if (!ctx) {
    return 0;
  }
  ctx->last_packet_id = (uint8_t)((ctx->last_packet_id + 1) & 0x7f);
  if (ctx->last_packet_id == 0) ctx->last_packet_id = 1;
  return ctx->last_packet_id;
}

uint8_t _srvint_compute_hdr_hash(uint8_t addr, uint8_t pid, uint8_t cmd,
                                 uint8_t length) {
  return addr ^ pid ^ cmd ^ length;
}

uint8_t _srvint_compute_payload_hash(const uint8_t *data, uint8_t length) {
  if (data == NULL || length == 0) {
    return 0;
  }
  uint8_t hash = 0;
  for (uint8_t i = 0; i < length; i++) {
    hash ^= data[i];
  }
  return hash;
}

int srvint_flush(srvint_t *ctx) {
  if (ctx == NULL || ctx->s < 0) { errno = EINVAL; return -EXIT_FAILURE; }
  return tcflush(ctx->s, TCIOFLUSH);
}

static int _srvint_build_request_header(srvint_t *ctx, int function,
                                        int payload_length, uint8_t *hdr) {
  if (ctx == NULL || hdr == NULL || ctx->slave == SRVINT_NULL_ADDRESS ||
      function < 0 || function > 255 || payload_length < 0 || payload_length > 255) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }
  uint8_t packet_id = _srvint_next_packet_id(ctx);
  hdr[0] = START_BYTE;
  hdr[1] = ctx->slave;
  hdr[2] = packet_id;
  hdr[3] = function;
  hdr[4] = payload_length;
  hdr[5] = _srvint_compute_hdr_hash(hdr[1], hdr[2], hdr[3], hdr[4]);
  return packet_id;
}

int srvint_get_header_length(srvint_t *ctx) {
  if (ctx == NULL) { errno = EINVAL; return -EXIT_FAILURE; }
  return SRVINT_HEADER_LENGTH;
}

static int read_some(srvint_t *ctx, uint8_t *dst, size_t length) {
  int timeout = (int)(ctx->response_timeout.tv_sec * 1000L +
                      (ctx->response_timeout.tv_usec + 999) / 1000);
  size_t got = 0;
  while (got < length) {
    struct pollfd pfd = {ctx->s, POLLIN, 0};
    int rc = poll(&pfd, 1, timeout);
    if (rc < 0 && errno == EINTR) continue;
    if (rc < 0) return -EXIT_FAILURE;
    if (rc == 0) { errno = ETIMEDOUT; return -EXIT_FAILURE; }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) { errno = EIO; return -EXIT_FAILURE; }
    ssize_t n = read(ctx->s, dst + got, length - got);
    if (n > 0) { got += (size_t)n; continue; }
    if (n < 0 && errno == EINTR) continue;
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
    if (n == 0) { errno = EIO; return -EXIT_FAILURE; }
    return -EXIT_FAILURE;
  }
  return (int)got;
}

static int receive_frame(srvint_t *ctx, uint8_t *frame, size_t capacity) {
  uint8_t byte;
  do {
    if (read_some(ctx, &byte, 1) < 0) return -EXIT_FAILURE;
  } while (byte != START_BYTE);
  frame[0] = byte;
  if (read_some(ctx, frame + 1, SRVINT_HEADER_LENGTH - 1) < 0) return -EXIT_FAILURE;
  if ((size_t)SRVINT_HEADER_LENGTH + frame[4] +
      (frame[4] != 0 ? 1 : 0) > capacity) {
    errno = ESIBADDATA;
    return -EXIT_FAILURE;
  }
  if (frame[5] != _srvint_compute_hdr_hash(frame[1], frame[2], frame[3], frame[4])) {
    errno = ESIBADCRC;
    return -EXIT_FAILURE;
  }
  if (frame[4] != 0) {
    if (read_some(ctx, frame + SRVINT_HEADER_LENGTH, frame[4] + 1) < 0) return -EXIT_FAILURE;
    if (frame[SRVINT_HEADER_LENGTH + frame[4]] !=
        _srvint_compute_payload_hash(frame + SRVINT_HEADER_LENGTH, frame[4])) {
      errno = ESIBADCRC;
      return -EXIT_FAILURE;
    }
  }
  return SRVINT_HEADER_LENGTH + frame[4] + (frame[4] != 0 ? 1 : 0);
}

int _srvint_recieve_msg(srvint_t *ctx, uint8_t *msg, msg_type_t msg_type) {
  (void)msg_type;
  if (ctx == NULL || msg == NULL || ctx->s < 0) { errno = EINVAL; return -EXIT_FAILURE; }
  return receive_frame(ctx, msg, SRVINT_HEADER_LENGTH + SRVINT_MAX_PAYLOAD + 1);
}

int srvint_request(srvint_t *ctx, uint8_t function, const uint8_t *request,
                   uint8_t request_length, uint8_t *response,
                   uint8_t response_capacity) {
  uint8_t frame[SRVINT_HEADER_LENGTH + SRVINT_MAX_PAYLOAD + 1];
  uint8_t reply[SRVINT_HEADER_LENGTH + SRVINT_MAX_PAYLOAD + 1];
  if (ctx == NULL || ctx->s < 0 || ctx->slave == SRVINT_NULL_ADDRESS ||
      (request_length != 0 && request == NULL) ||
      (response_capacity != 0 && response == NULL)) {
    errno = EINVAL;
    return -EXIT_FAILURE;
  }
  int packet_id = _srvint_build_request_header(ctx, function, request_length, frame);
  if (packet_id < 0) return -EXIT_FAILURE;
  if (request_length != 0) memcpy(frame + SRVINT_HEADER_LENGTH, request, request_length);
  if (request_length != 0) {
    frame[SRVINT_HEADER_LENGTH + request_length] =
        _srvint_compute_payload_hash(request, request_length);
  }
  if (send_msg(ctx, frame, SRVINT_HEADER_LENGTH + request_length +
               (request_length != 0 ? 1 : 0)) < 0) return -EXIT_FAILURE;
  if (receive_frame(ctx, reply, sizeof(reply)) < 0) return -EXIT_FAILURE;
  if (reply[1] != SRVINT_MASTER_ADDRESS || reply[2] != (uint8_t)packet_id ||
      reply[3] != function) {
    errno = ESIBADDATA;
    return -EXIT_FAILURE;
  }
  if (reply[4] > response_capacity) { errno = EMSGSIZE; return -EXIT_FAILURE; }
  if (reply[4] != 0) memcpy(response, reply + SRVINT_HEADER_LENGTH, reply[4]);
  return reply[4];
}
