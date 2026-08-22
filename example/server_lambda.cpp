#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <srvint.h>

struct ApplicationState {
  std::uint8_t last_command = 0;
};

int main(int argc, char **argv) {
  const char *device = argc > 1 ? argv[1] : "/dev/ttyUSB0";
  const int slave_address = argc > 2 ? std::atoi(argv[2]) : SRVINT_DEVICE_ADDRESS;

  srvint_server_t *server =
      ::srvint_serial_server_new(device, 115200, 'N', 8, 1);
  if (server == nullptr) {
    std::cerr << "unable to create SrvInt server context\n";
    return EXIT_FAILURE;
  }


  ApplicationState state;

  // A non-capturing C++ lambda converts to the C callback function pointer.
  const srvint_server_handler_t handler =
      [](srvint_server_t *, std::uint8_t, std::uint8_t, std::uint8_t command,
         const std::uint8_t *request, std::uint8_t request_length,
         std::uint8_t *response_command, std::uint8_t *response,
         std::uint8_t response_capacity, std::uint8_t *response_length,
         void *user_data) -> int {
    auto *application = static_cast<ApplicationState *>(user_data);
    application->last_command = command;

    if (command == SRVINT_FC_PING) {
      if (response_capacity < 1) return -1;
      response[0] = 0;
      *response_length = 1;
      return SRVINT_SERVER_REPLY;
    }

    if (command == SRVINT_FC_GET_PARAM && request_length >= 2) {
      // request[0] is the application-defined group and request[1] is its
      // parameter code. The application owns all value decoding here.
      if (response_capacity < 1) return -1;
      response[0] = 0;
      *response_length = 1;
      return SRVINT_SERVER_REPLY;
    }

    if (command == 0x99) {
      *response_command = SRVINT_FC_UNKNOWN;
      if (response_capacity < 1) return -1;
      response[0] = 0;
      *response_length = 1;
      return SRVINT_SERVER_REPLY;
    }

    return SRVINT_SERVER_NO_REPLY;
  };

  if (::srvint_server_set_slave(server, slave_address) != 0 ||
      ::srvint_server_connect(server) != 0) {
    std::cerr << "unable to connect SrvInt server to " << device << '\n';
    ::srvint_server_free(server);
    return EXIT_FAILURE;
  }

  std::cout << "SrvInt server listening on " << device
            << ", slave address " << slave_address << '\n';
  const int result = ::srvint_server_run(server, handler, &state);

  ::srvint_server_close(server);
  ::srvint_server_free(server);
  return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
