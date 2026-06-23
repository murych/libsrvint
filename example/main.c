/*!
 *
 */

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include "srvint.h"

int main(int argc, char** argv) {
  srvint_t* ctx = NULL;
  int ret = 0;

  ctx = srvint_serial_new("/dev/ttyUSB0", 115200, 'N', 8, 1);
  if (ctx == NULL) {
    perror("unable to create libsrvint context\n");
    perror(srvint_strerror(errno));
    return -1;
  }
  ret = srvint_set_debug(ctx, TRUE);
  ret = srvint_set_slave(ctx, SRVINT_DEVICE_ADDRESS);
  ret = srvint_connect(ctx);
  if (ret < EXIT_SUCCESS) {
    perror("srvint connect error\n");
    perror(srvint_strerror(errno));
    return -1;
  }

  srvint_close(ctx);
  srvint_free(ctx);

  perror("all ok");
  return 0;
}
