# libsrvint

## Server emulation

The server API provides serial transport and SrvInt frame handling without
imposing a parameter storage model. Applications decode `GET_PARAM` and
`SET_PARAM` operands themselves.

```c
static int handle_params(
    srvint_server_t *server, const uint8_t *request, size_t request_length,
    uint8_t *response, size_t response_capacity, size_t *response_length,
    void *user_data) {
  (void)server; (void)user_data;

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

Create a transport with `srvint_serial_new`, configure debug or other transport
options, and transfer ownership to `srvint_server_new`. The application then
controls the loop:

```c
srvint_t *transport = srvint_serial_new("/dev/ttyUSB0", 115200, 'N', 8, 1);
srvint_set_debug(transport, TRUE);
srvint_server_t *server = srvint_server_new(transport);
srvint_server_set_slave(server, SRVINT_DEVICE_ADDRESS);
srvint_server_connect(server);

uint8_t request[6 + 255 + 1];
for (;;) {
  int length = srvint_server_receive(server, request, sizeof(request));
  if (length < 0) continue;
  if (srvint_server_reply(server, request, (size_t)length,
                          handle_params, NULL) < 0) break;
}

srvint_server_close(server);
srvint_server_free(server); /* also frees transport */
```

`receive()` returns a complete validated wire frame. `reply()` handles the
standard commands and invokes the callback for raw `GET_PARAM`/`SET_PARAM`
frames. Broadcast requests invoke the callback but never receive a response.
