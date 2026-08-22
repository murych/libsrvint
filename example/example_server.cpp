#include <srvint.h>

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <iostream>

struct ApplicationState {
  std::uint8_t last_command = 0;
};

int main(int argc, char **argv) {
  const char *device = argc > 1 ? argv[1] : "/dev/ttyUSB0";
  const int slave_address =
      argc > 2 ? std::atoi(argv[2]) : SRVINT_DEVICE_ADDRESS;

  srvint_t *ctx = ::srvint_serial_new(device, 115200, 'N', 8, 1);
  if (ctx == nullptr) {
    std::cerr << "unable to create SrvInt transport context\n";
    return EXIT_FAILURE;
  }
  ::srvint_set_debug(ctx, TRUE);

  ApplicationState state;

  // A non-capturing C++ lambda converts to the C parameter callback pointer.
  const srvint_param_callback_t handler =
      [](srvint_t *, const std::uint8_t *request, std::size_t request_length,
         std::uint8_t *response, std::size_t response_capacity,
         std::size_t *response_length, void *user_data) -> int {
    auto *application = static_cast<ApplicationState *>(user_data);
    if (request_length < 6 || request[4] < 2 || response_capacity < 1) {
      return -1;
    }
    application->last_command = request[3];

    // request[6] is the application-defined group and request[7] is its
    // parameter code. The application owns all value decoding here.
    if (request[3] == SRVINT_FC_GET_PARAM ||
        request[3] == SRVINT_FC_SET_PARAM) {
      response[0] = 0;
      *response_length = 1;
      return 0;
    }

    return -1;
  };

  if (::srvint_set_slave(ctx, slave_address) != EXIT_SUCCESS ||
      ::srvint_connect(ctx) != EXIT_SUCCESS) {
    std::cerr << "unable to connect SrvInt server to " << device << '\n';
    ::srvint_free(ctx);
    return EXIT_FAILURE;
  }

  std::cout << "SrvInt server listening on " << device << ", slave address "
            << slave_address << '\n';
  constexpr std::size_t max_frame_length = 6 + 255 + 1;
  std::array<std::uint8_t, max_frame_length> request{};
  int result = EXIT_SUCCESS;
  for (;;) {
    const int request_length =
        ::srvint_receive(ctx, request.data(), sizeof(request));
    if (request_length < 0) {
      if (errno == ETIMEDOUT) {
        continue;
      }
      result = EXIT_FAILURE;
      break;
    }
    if (::srvint_reply(ctx, request.data(),
                       static_cast<std::size_t>(request_length), handler,
                       &state) != EXIT_SUCCESS) {
      result = EXIT_FAILURE;
      break;
    }
  }

  ::srvint_close(ctx);
  ::srvint_free(ctx);
  return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
