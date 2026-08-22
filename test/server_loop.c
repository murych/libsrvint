#include <poll.h>
#include <pty.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "srvint.h"

#define TEST_HEADER_LENGTH 6
#define TEST_MAX_PAYLOAD 255

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

static int send_frame(int fd, uint8_t address, uint8_t packet_id,
                      uint8_t command, const uint8_t *payload,
                      uint8_t payload_length, int corrupt_header_hash) {
  uint8_t frame[TEST_HEADER_LENGTH + TEST_MAX_PAYLOAD + 1];
  frame[0] = SRVINT_START_BYTE;
  frame[1] = address;
  frame[2] = packet_id;
  frame[3] = command;
  frame[4] = payload_length;
  frame[5] = xor4(frame[1], frame[2], frame[3], frame[4]);
  if (corrupt_header_hash) frame[5] ^= 1;
  if (payload_length != 0) {
    for (uint8_t i = 0; i < payload_length; ++i) frame[6 + i] = payload[i];
    frame[6 + payload_length] = xor_data(payload, payload_length);
  }
  return write_all(fd, frame, 6 + payload_length + (payload_length != 0));
}

static int read_frame(int fd, uint8_t *frame, size_t capacity) {
  if (read_all(fd, frame, 6) != 0) return -1;
  if ((size_t)6 + frame[4] + (frame[4] != 0) > capacity) return -1;
  if (frame[4] != 0 && read_all(fd, frame + 6, frame[4] + 1) != 0) return -1;
  return 0;
}

static int handler(srvint_server_t *server, uint8_t address, uint8_t packet_id,
                   uint8_t command, const uint8_t *request,
                   uint8_t request_length, uint8_t *response_command,
                   uint8_t *response, uint8_t response_capacity,
                   uint8_t *response_length, void *user_data) {
  (void)server;
  (void)packet_id;
  (void)user_data;
  if (address == SRVINT_BROADCAST_ADDRESS) {
    *response_length = 1;
    response[0] = 0xee;
    return SRVINT_SERVER_REPLY;
  }
  if (command == 0x81) {
    *response_length = 1;
    response[0] = 0x42;
    return SRVINT_SERVER_REPLY;
  }
  if (command == 0x86 && request_length <= response_capacity) {
    for (uint8_t i = 0; i < request_length; ++i) response[i] = request[i];
    *response_length = request_length;
    return SRVINT_SERVER_REPLY;
  }
  if (command == 0x99) {
    *response_command = SRVINT_FC_UNKNOWN;
    *response_length = 1;
    response[0] = 0x77;
    return SRVINT_SERVER_REPLY;
  }
  if (command == 0x90) {
    *response_length = 1;
    response[0] = 0x90;
    return SRVINT_SERVER_STOP;
  }
  return -1;
}

static int run_server(const char *device, int ready_fd) {
  srvint_server_t *server = srvint_serial_server_new(device, 115200, 'N', 8, 1);
  if (server == NULL || srvint_server_set_slave(server, 0x10) != 0 ||
      srvint_server_connect(server) != 0) {
    srvint_server_free(server);
    return 10;
  }
  if (write(ready_fd, "R", 1) != 1) return 12;
  close(ready_fd);
  int result = srvint_server_run(server, handler, NULL);
  srvint_server_close(server);
  srvint_server_free(server);
  return result == 0 ? 0 : 11;
}

int main(void) {
  int master, slave;
  char device[128];
  int ready[2];
  if (openpty(&master, &slave, device, NULL, NULL) != 0) return EXIT_FAILURE;
  if (pipe(ready) != 0) return EXIT_FAILURE;

  pid_t server = fork();
  if (server < 0) return EXIT_FAILURE;
  if (server == 0) {
    close(master);
    close(ready[0]);
    _exit(run_server(device, ready[1]));
  }
  close(ready[1]);
  char ready_byte;
  if (read(ready[0], &ready_byte, 1) != 1) return EXIT_FAILURE;
  close(ready[0]);

  uint8_t frame[262];
  uint8_t payload[] = {0xb0, 0x01, 0x42};
  if (send_frame(master, 0x10, 1, 0x81, NULL, 0, 1) != 0 ||
      send_frame(master, 0x10, 2, 0x81, NULL, 0, 0) != 0 ||
      read_frame(master, frame, sizeof(frame)) != 0 || frame[1] != 0xca ||
      frame[2] != 2 || frame[3] != 0x81 || frame[4] != 1 || frame[6] != 0x42 ||
      send_frame(master, 0x10, 3, 0x86, payload, sizeof(payload), 0) != 0 ||
      read_frame(master, frame, sizeof(frame)) != 0 || frame[3] != 0x86 ||
      frame[4] != sizeof(payload) || frame[6] != payload[0] ||
      send_frame(master, 0x10, 4, 0x99, NULL, 0, 0) != 0 ||
      read_frame(master, frame, sizeof(frame)) != 0 || frame[3] != 0x00 ||
      frame[6] != 0x77) {
    return EXIT_FAILURE;
  }

  if (send_frame(master, SRVINT_BROADCAST_ADDRESS, 5, 0x86, payload,
                 sizeof(payload), 0) != 0) return EXIT_FAILURE;
  struct pollfd poll_fd = {master, POLLIN, 0};
  if (poll(&poll_fd, 1, 100) != 0) return EXIT_FAILURE;

  if (send_frame(master, 0x10, 6, 0x90, NULL, 0, 0) != 0 ||
      read_frame(master, frame, sizeof(frame)) != 0 || frame[3] != 0x90 ||
      frame[6] != 0x90) return EXIT_FAILURE;

  int status;
  close(slave);
  waitpid(server, &status, 0);
  return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? EXIT_SUCCESS
                                                       : EXIT_FAILURE;
}
