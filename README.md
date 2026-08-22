# libsrvint

## Server emulation

The server API provides serial transport and SrvInt frame handling without
imposing a parameter storage model. Applications decode `GET_PARAM` and
`SET_PARAM` operands themselves.

```c
static int handle_params(
    srvint_t *ctx, const uint8_t *request, size_t request_length,
    uint8_t *response, size_t response_capacity, size_t *response_length,
    void *user_data) {
  (void)ctx; (void)user_data;

  if (request_length >= 9 && request[4] >= 2) {
    /* request[6] = group, request[7] = parameter; decode raw value bytes here. */
    if (response_capacity < 1) return -1;
    response[0] = 0; /* application-defined raw value */
    *response_length = 1;
    return 0;
  }

  return -1;
}
```

Create a transport with `srvint_serial_new`, configure it, and use the same
opaque context for the server receive/reply loop:

```c
srvint_t *ctx = srvint_serial_new("/dev/ttyUSB0", 115200, 'N', 8, 1);
srvint_set_debug(ctx, TRUE);
srvint_set_slave(ctx, SRVINT_DEVICE_ADDRESS);
srvint_connect(ctx);

uint8_t request[6 + 255 + 1];
for (;;) {
  int length = srvint_receive(ctx, request, sizeof(request));
  if (length < 0) continue;
  if (srvint_reply(ctx, request, (size_t)length,
                          handle_params, NULL) < 0) break;
}

srvint_close(ctx);
srvint_free(ctx);
```

`receive()` returns a complete validated wire frame. `reply()` handles the
standard commands and invokes the callback for raw `GET_PARAM`/`SET_PARAM`
frames. Broadcast requests invoke the callback but never receive a response.
