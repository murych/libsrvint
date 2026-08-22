# libsrvint

## Server emulation

The server API provides serial transport and SrvInt frame handling without
imposing a parameter storage model. Applications decode `GET_PARAM` and
`SET_PARAM` operands themselves.

```c
static int handle_request(
    srvint_server_t *server, uint8_t address, uint8_t packet_id,
    uint8_t command, const uint8_t *request, uint8_t request_length,
    uint8_t *response_command, uint8_t *response, uint8_t response_capacity,
    uint8_t *response_length, void *user_data) {
  (void)server; (void)address; (void)packet_id; (void)user_data;

  if (command == SRVINT_FC_GET_PARAM && request_length >= 2) {
    /* request[0] = group, request[1] = parameter; decode the rest here. */
    if (response_capacity < 1) return -1;
    response[0] = 0; /* application-defined raw value */
    *response_length = 1;
    return SRVINT_SERVER_REPLY;
  }

  if (command == 0x99) {
    *response_command = SRVINT_FC_UNKNOWN;
    response[0] = 0;
    *response_length = 1;
    return SRVINT_SERVER_REPLY;
  }

  return SRVINT_SERVER_NO_REPLY;
}
```

Create a server with `srvint_serial_server_new`, configure its Slave address,
connect it, and call `srvint_server_run`. The callback receives raw operands;
the library adds framing and checksums. Broadcast requests invoke the callback
but never receive a response.
