#include <poll.h>
#include <pty.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "srvint.h"

#define HEADER_LENGTH 6
#define MAX_FRAME_LENGTH (HEADER_LENGTH + 255 + 1)

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
  uint8_t frame[MAX_FRAME_LENGTH];
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
  return write_all(fd, frame, HEADER_LENGTH + payload_length +
                              (payload_length != 0 ? 1 : 0));
}

static int send_bad_data_frame(int fd) {
  uint8_t frame[8] = {SRVINT_START_BYTE, 0x10, 0x7e, SRVINT_FC_PING,
                      1, 0, 0xaa, 0};
  frame[5] = xor4(frame[1], frame[2], frame[3], frame[4]);
  frame[7] = (uint8_t)(frame[6] ^ 1);
  return write_all(fd, frame, sizeof(frame));
}

static int read_frame(int fd, uint8_t *frame, size_t capacity) {
  if (read_all(fd, frame, HEADER_LENGTH) != 0) return -1;
  size_t length = HEADER_LENGTH + frame[4] + (frame[4] != 0 ? 1 : 0);
  if (length > capacity) return -1;
  if (frame[4] != 0 && read_all(fd, frame + HEADER_LENGTH, frame[4] + 1) != 0) {
    return -1;
  }
  return (int)length;
}

static int parameter_callback(srvint_server_t *server, const uint8_t *request,
                              size_t request_length, uint8_t *response,
                              size_t response_capacity, size_t *response_length,
                              void *user_data) {
  (void)server;
  uint8_t *callback_count = (uint8_t *)user_data;
  if (request_length < HEADER_LENGTH || request[4] < 2 ||
      request_length < HEADER_LENGTH + request[4] + 1 ||
      response_capacity < 2) {
    return -1;
  }
  ++*callback_count;
  response[0] = request[6];
  response[1] = request[7];
  *response_length = 2;
  return 0;
}

static int run_server(const char *device, int ready_fd) {
  srvint_t *transport = srvint_serial_new(device, 115200, 'N', 8, 1);
  srvint_server_t *server;
  uint8_t request[MAX_FRAME_LENGTH];
  uint8_t callback_count = 0;
  int served = 0;

  if (transport == NULL) return 10;
  server = srvint_server_new(transport);
  if (server == NULL) {
    srvint_free(transport);
    return 11;
  }
  if (srvint_server_set_slave(server, 0x10) != 0 ||
      srvint_server_set_last_error(server, 0x42) != 0 ||
      srvint_server_set_error(server, 7, 0x99) != 0 ||
      srvint_server_connect(server) != 0) {
    srvint_server_free(server);
    return 12;
  }
  if (write(ready_fd, "R", 1) != 1) return 13;
  close(ready_fd);

  while (served < 9) {
    int length = srvint_server_receive(server, request, sizeof(request));
    if (length < 0) {
      if (errno == ESIBADCRC || errno == ESIBADDATA || errno == ETIMEDOUT) {
        continue;
      }
      srvint_server_free(server);
      return 14;
    }
    int reply_result = srvint_server_reply(server, request, (size_t)length,
                                           parameter_callback, &callback_count);
    if (reply_result != 0) {
      srvint_server_free(server);
      return 15;
    }
    ++served;
  }

  srvint_server_close(server);
  srvint_server_free(server);
  return callback_count == 3 ? 0 : 16;
}

static int expect_response(int fd, uint8_t *frame, uint8_t packet_id,
                           uint8_t command, uint8_t payload_length) {
  int length = read_frame(fd, frame, MAX_FRAME_LENGTH);
  return length >= HEADER_LENGTH && frame[1] == SRVINT_MASTER_ADDRESS &&
                 frame[2] == packet_id && frame[3] == command &&
                 frame[4] == payload_length ? 0 : -1;
}

int main(void) {
  int master, slave, ready[2];
  char device[128];
  uint8_t frame[MAX_FRAME_LENGTH];
  uint8_t parameter[] = {0xb0, 0x01, 0x42};
  char ready_byte;

  if (openpty(&master, &slave, device, NULL, NULL) != 0 || pipe(ready) != 0) {
    return EXIT_FAILURE;
  }
  pid_t child = fork();
  if (child < 0) return EXIT_FAILURE;
  if (child == 0) {
    close(master);
    close(ready[0]);
    _exit(run_server(device, ready[1]));
  }
  close(ready[1]);
  if (read(ready[0], &ready_byte, 1) != 1) return EXIT_FAILURE;
  close(ready[0]);

  if (send_frame(master, 0x10, 1, SRVINT_FC_PING, NULL, 0, 1) != 0 ||
      send_bad_data_frame(master) != 0 ||
      send_frame(master, 0x10, 0x80, SRVINT_FC_PING, NULL, 0, 0) != 0 ||
      send_frame(master, 0x10, 2, SRVINT_FC_PING, NULL, 0, 0) != 0 ||
      expect_response(master, frame, 2, SRVINT_FC_PING, 1) != 0 ||
      frame[6] != 0x42 ||
      send_frame(master, 0x10, 3, SRVINT_FC_HW_RESET, NULL, 0, 0) != 0 ||
      expect_response(master, frame, 3, SRVINT_FC_HW_RESET, 1) != 0 ||
      frame[6] != 0x42 ||
      send_frame(master, 0x10, 4, SRVINT_FC_SW_RESET, NULL, 0, 0) != 0 ||
      expect_response(master, frame, 4, SRVINT_FC_SW_RESET, 1) != 0 ||
      frame[6] != 0x42 ||
      send_frame(master, 0x10, 5, SRVINT_FC_GET_ERROR, (uint8_t[]){7}, 1, 0) != 0 ||
      expect_response(master, frame, 5, SRVINT_FC_GET_ERROR, 3) != 0 ||
      frame[6] != 7 || frame[7] != 0x99 || frame[8] != 0x42 ||
      send_frame(master, 0x10, 6, SRVINT_FC_GET_PARAM, parameter, 3, 0) != 0 ||
      expect_response(master, frame, 6, SRVINT_FC_GET_PARAM, 2) != 0 ||
      frame[6] != 0xb0 || frame[7] != 1 ||
      send_frame(master, 0x10, 7, SRVINT_FC_SET_PARAM, parameter, 3, 0) != 0 ||
      expect_response(master, frame, 7, SRVINT_FC_SET_PARAM, 2) != 0 ||
      send_frame(master, 0x10, 8, 0x99, NULL, 0, 0) != 0 ||
      expect_response(master, frame, 8, SRVINT_FC_UNKNOWN, 1) != 0 ||
      frame[6] != 0x42 ||
      send_frame(master, SRVINT_BROADCAST_ADDRESS, 9, SRVINT_FC_SET_PARAM,
                 parameter, 3, 0) != 0) {
    return EXIT_FAILURE;
  }

  struct pollfd poll_fd = {master, POLLIN, 0};
  if (poll(&poll_fd, 1, 100) != 0 ||
      send_frame(master, 0x10, 10, SRVINT_FC_ZEROIZE_ERROR, NULL, 0, 0) != 0 ||
      expect_response(master, frame, 10, SRVINT_FC_ZEROIZE_ERROR, 1) != 0 ||
      frame[6] != 0) {
    return EXIT_FAILURE;
  }

  close(slave);
  int status;
  waitpid(child, &status, 0);
  return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? EXIT_SUCCESS
                                                       : EXIT_FAILURE;
}
