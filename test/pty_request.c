#include <pty.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "srvint.h"

static uint8_t xor4(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  return (uint8_t)(a ^ b ^ c ^ d);
}

static uint8_t xor_data(const uint8_t *data, size_t length) {
  uint8_t hash = 0;
  while (length-- != 0) {
    hash ^= *data++;
  }
  return hash;
}

int main(void) {
  int master;
  int slave;
  char device[128];

  if (openpty(&master, &slave, device, NULL, NULL) < 0) {
    perror("openpty");
    return EXIT_FAILURE;
  }

  pid_t server = fork();
  if (server < 0) {
    perror("fork");
    return EXIT_FAILURE;
  }

  if (server == 0) {
    uint8_t request[7];
    uint8_t response[9];

    close(slave);
    if (read(master, request, sizeof(request)) != (ssize_t)sizeof(request)) {
      _exit(2);
    }

    if (request[0] != SRVINT_START_BYTE || request[1] != 0x10 ||
        (request[2] & 0x80) != 0 || request[3] != SRVINT_FC_PING ||
        request[4] != 1 ||
        request[5] != xor4(request[1], request[2], request[3], request[4]) ||
        request[6] != 0x42) {
      _exit(3);
    }

    response[0] = SRVINT_START_BYTE;
    response[1] = SRVINT_MASTER_ADDRESS;
    response[2] = request[2];
    response[3] = request[3];
    response[4] = 2;
    response[5] = xor4(response[1], response[2], response[3], response[4]);
    response[6] = 0xab;
    response[7] = 0xcd;
    response[8] = xor_data(response + 6, 2);

    if (write(master, response, sizeof(response)) !=
        (ssize_t)sizeof(response)) {
      _exit(4);
    }
    _exit(0);
  }

  close(master);

  srvint_t *ctx = srvint_serial_new(device, 115200, 'N', 8, 1);
  if (ctx == NULL || srvint_set_slave(ctx, 0x10) != 0 ||
      srvint_set_response_timeout(ctx, 1, 0) != 0 || srvint_connect(ctx) != 0) {
    srvint_free(ctx);
    return EXIT_FAILURE;
  }

  uint8_t request = 0x42;
  uint8_t response[2] = {0};
  int response_length = srvint_request(ctx, SRVINT_FC_PING, &request, 1,
                                       response, sizeof(response));
  srvint_close(ctx);
  srvint_free(ctx);

  int status;
  waitpid(server, &status, 0);
  if (response_length != 2 || response[0] != 0xab || response[1] != 0xcd ||
      !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
    return EXIT_FAILURE;
  }

  puts("pty request test: ok");
  return EXIT_SUCCESS;
}
