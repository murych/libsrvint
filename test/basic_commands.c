#include <pty.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "srvint.h"

static uint8_t xor4(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  return (uint8_t)(a ^ b ^ c ^ d);
}

static uint8_t xor_data(const uint8_t *data, size_t length) {
  uint8_t hash = 0;
  while (length-- != 0) hash ^= *data++;
  return hash;
}

static int read_all(int fd, uint8_t *data, size_t length) {
  size_t offset = 0;
  while (offset < length) {
    ssize_t count = read(fd, data + offset, length - offset);
    if (count <= 0) return -1;
    offset += (size_t)count;
  }
  return 0;
}

static int write_all(int fd, const uint8_t *data, size_t length) {
  size_t offset = 0;
  while (offset < length) {
    ssize_t count = write(fd, data + offset, length - offset);
    if (count <= 0) return -1;
    offset += (size_t)count;
  }
  return 0;
}

static int server_loop(int fd) {
  for (int command_number = 0; command_number < 8; ++command_number) {
    uint8_t request[262] = {0};
    uint8_t response[262] = {0};

    if (read_all(fd, request, 6) != 0) return 2;
    size_t request_length = request[4];
    if (request_length != 0 &&
        read_all(fd, request + 6, request_length + 1) != 0) return 3;

    uint8_t response_length;
    uint8_t response_command = request[3];
    switch (request[3]) {
      case SRVINT_FC_PING: response_length = 1; response[6] = 0x11; break;
      case SRVINT_FC_HW_RESET: response_length = 1; response[6] = 0x22; break;
      case SRVINT_FC_SW_RESET: response_length = 1; response[6] = 0x33; break;
      case SRVINT_FC_GET_ERROR:
        if (request_length != 1) return 4;
        response_length = 3;
        response[6] = request[6]; response[7] = 0x44; response[8] = 0x55;
        break;
      case SRVINT_FC_ZEROIZE_ERROR: response_length = 1; response[6] = 0x66; break;
      case 0x99:
        if (request_length != 2) return 5;
        response_command = SRVINT_FC_UNKNOWN;
        response_length = 1; response[6] = 0x77;
        break;
      case SRVINT_FC_SET_PARAM:
        if (request_length != 3 || request[6] != 0xb0) return 8;
        response_length = 4;
        response[6] = request[6]; response[7] = request[7];
        response[8] = request[8]; response[9] = 0x88;
        break;
      case SRVINT_FC_GET_PARAM:
        if (request_length != 2 || request[6] != 0xb2) return 9;
        response_length = 3;
        response[6] = request[6]; response[7] = request[7];
        response[8] = 0x99;
        break;
      default: return 6;
    }

    response[0] = SRVINT_START_BYTE;
    response[1] = SRVINT_MASTER_ADDRESS;
    response[2] = request[2];
    response[3] = response_command;
    response[4] = response_length;
    response[5] = xor4(response[1], response[2], response[3], response[4]);
    if (response_length != 0) {
      response[6 + response_length] = xor_data(response + 6, response_length);
    }
    if (write_all(fd, response, 6 + response_length + 1) != 0) return 7;
  }
  return 0;
}

int main(void) {
  int master, slave;
  char device[128];
  if (openpty(&master, &slave, device, NULL, NULL) != 0) return EXIT_FAILURE;

  pid_t server = fork();
  if (server < 0) return EXIT_FAILURE;
  if (server == 0) {
    close(slave);
    _exit(server_loop(master));
  }
  close(master);

  srvint_t *ctx = srvint_serial_new(device, 115200, 'N', 8, 1);
  if (ctx == NULL || srvint_set_slave(ctx, 0x10) != 0 ||
      srvint_set_response_timeout(ctx, 1, 0) != 0 || srvint_connect(ctx) != 0) {
    srvint_free(ctx); return EXIT_FAILURE;
  }

  uint8_t value, error_value, last_error;
  uint8_t unknown_operands[2] = {0xaa, 0xbb};
  int result = srvint_ping(ctx, &value) || value != 0x11;
  result |= srvint_hw_reset(ctx, &value) || value != 0x22;
  result |= srvint_sw_reset(ctx, &value) || value != 0x33;
  result |= srvint_get_error(ctx, 7, &error_value, &last_error) ||
            error_value != 0x44 || last_error != 0x55;
  result |= srvint_zeroize_error(ctx, &value) || value != 0x66;
  result |= srvint_unknown(ctx, 0x99, unknown_operands, sizeof(unknown_operands), &value) ||
            value != 0x77;
  uint8_t set_request[3] = {0xb0, 0x01, 0x42};
  uint8_t set_response[4] = {0};
  int set_length = srvint_set_param(ctx, set_request, sizeof(set_request),
                                    set_response, sizeof(set_response));
  result |= set_length != 4 || set_response[0] != 0xb0 ||
            set_response[3] != 0x88;

  uint8_t get_request[2] = {0xb2, 0x03};
  uint8_t get_response[3] = {0};
  int get_length = srvint_get_param(ctx, get_request, sizeof(get_request),
                                    get_response, sizeof(get_response));
  result |= get_length != 3 || get_response[0] != 0xb2 ||
            get_response[2] != 0x99;

  srvint_close(ctx); srvint_free(ctx);
  int status;
  waitpid(server, &status, 0);
  if (result != 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
