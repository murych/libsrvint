#include <srvint.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  const char* device = argc > 1 ? argv[1] : "/dev/ttyUSB0";
  const int slave_address =
      argc > 2 ? std::atoi(argv[2]) : SRVINT_DEVICE_ADDRESS;

  auto* ctx = ::srvint_serial_new(device, 115200, 'N', 8, 1);
  if (ctx == nullptr) {
    std::cerr << "unable to create srvint serial context'\n";
    return EXIT_FAILURE;
  }

  ::srvint_set_debug(ctx, true);
  if (::srvint_set_slave(ctx, slave_address) != EXIT_SUCCESS ||
      ::srvint_connect(ctx) != EXIT_SUCCESS) {
    std::cerr << "unable to connect to srvint server to " << device << '\n';
    ::srvint_free(ctx);
    return EXIT_FAILURE;
  }

  std::cout << "srvint connected to server on " << device << ", slave address "
            << slave_address << '\n';
  std::uint8_t last_error{0u};
  int res{0};

  for (int i = 0; i < 10; i++) {
    res = ::srvint_ping(ctx, &last_error);
    if (res != EXIT_SUCCESS) {
      auto saved_errno = errno;
      std::cerr << "failed to perform ping on server " << slave_address
                << ", error code " << saved_errno << " "
                << ::srvint_strerror(saved_errno) << ", last error code "
                << static_cast<int>(last_error) << '\n';
      break;
    }
    if (last_error != 0) {
      std::cerr << "recieved error code from server " << slave_address
                << ", code " << static_cast<int>(last_error) << '\n';
      last_error = 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{500});
  }

  ::srvint_close(ctx);
  ::srvint_free(ctx);
  return EXIT_SUCCESS;
}
