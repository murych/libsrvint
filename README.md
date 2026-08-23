# libsrvint

`libsrvint` - библиотека на C99 для обмена по протоколу SrvInt через
последовательный порт в POSIX-совместимых системах.

Библиотека предоставляет:

- настройку и управление serial-портом;
- формирование и разбор SrvInt-кадров;
- проверку header/data checksum;
- клиентский API в режиме запрос-ответ;
- server-style API `srvint_receive()` / `srvint_reply()`;
- поддержку raw-операндов для `GET_PARAM` и `SET_PARAM`.

Хранение параметров и преобразование их типов намеренно не входят в библиотеку. 
Приложение самостоятельно определяет, как интерпретировать байты параметров: например, как `uint8_t`, `uint32_t`, `int64_t`, `double` или структуру собственного формата.

## Состояние реализации

Основной режим работы SrvInt - синхронный обмен `Master -> Slave -> Master`.
Клиентская часть и server-style API работают с serial transport и opaque-контекстом `srvint_t`.

Server-style API не содержит register mapping.
Приложение вызывает `srvint_receive()`, анализирует полный проверенный кадр и передаёт его в `srvint_reply()`. 
Для `GET_PARAM` и `SET_PARAM` библиотека вызывает callback, который формирует raw response payload.

Асинхронные packet ID с установленным bit 7 в первой реализации server API не поддерживаются и отклоняются.

## Сборка

Требования:

- CMake 3.14 или новее;
- компилятор с поддержкой C99;
- POSIX-совместимая система для serial transport;
- C++23 требуется только для C++ example.

### Сборка библиотеки

```sh
cmake -S . -B build
cmake --build build
```

### Сборка тестов и примеров

Тесты используют pseudo-terminal и доступны в developer mode:

```sh
cmake -S . -B build \
  -Dlibsrvint_DEVELOPER_MODE=ON \
  -DLIBSRVINT_BUILD_TESTS=ON \
  -DLIBSRVINT_BUILD_EXAMPLES=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Собираемые примеры находятся в `example/`:

- `example_server` - server-style обработка с C++ lambda callback;
- `example_client` - пример клиентского обмена.

Для установки библиотеки можно использовать стандартную CMake-команду:

```sh
cmake --install build --prefix /usr/local
```

### Start byte

В спецификации протокола в качестве `START_BYTE` указан `0xB7`. 
В данной реализации используется фактическое значение `0x55` по умолчанию. 
Значение можно изменить при конфигурации:

```sh
cmake -S . -B build-b7 -DSRVINT_START_BYTE=0xB7
cmake --build build-b7
```

Одинаковое значение `SRVINT_START_BYTE` должно использоваться всеми участниками обмена.

## Формат SrvInt-кадра

Каждый кадр имеет следующий wire format:

```text
START_BYTE ADR PACKET_ID CMD DATA_COUNT HASH [OP_1 ... OP_N DATA_HASH]
```

| Поле | Размер | Описание |
| --- | ---: | --- |
| `START_BYTE` | 1 байт | Начало кадра. По умолчанию `0x55`, задаётся через `SRVINT_START_BYTE`. |
| `ADR` | 1 байт | Адрес назначения в запросе или адрес отправителя в ответе. |
| `PACKET_ID` | 1 байт | Идентификатор пакета. Bit 7 обозначает asynchronous packet; server API его отклоняет. |
| `CMD` | 1 байт | Код команды. |
| `DATA_COUNT` | 1 байт | Количество operand bytes, от 0 до 255. |
| `HASH` | 1 байт | XOR от `ADR`, `PACKET_ID`, `CMD` и `DATA_COUNT`. |
| `OP_1 ... OP_N` | 0-255 байт | Операнды или данные команды. |
| `DATA_HASH` | 0 или 1 байт | XOR операндов. Отсутствует при `DATA_COUNT == 0`. |

Размер заголовка равен 6 байтам. Максимальный размер полного кадра:

```text
6 + 255 + 1 = 262 байта
```

При `DATA_COUNT == 0` кадр заканчивается полем `HASH`.
При ненулевом `DATA_COUNT` после операндов передаётся `DATA_HASH`.

### Адреса

| Константа | Значение | Назначение |
| --- | ---: | --- |
| `SRVINT_NULL_ADDRESS` | `0x00` | Зарезервированный/null address. |
| `SRVINT_DEVICE_ADDRESS` | `0x10` | Адрес устройства по умолчанию в API. |
| `SRVINT_BROADCAST_ADDRESS` | `0x55` | Broadcast-запрос. Callback выполняется, ответ не отправляется. |
| `SRVINT_MASTER_ADDRESS` | `0xCA` | Адрес Master в ответных кадрах. |

Server-style receive принимает кадры с адресом локального Slave или с broadcast-адресом.
Ответы всегда формируются с адресом `0xCA` и сохраняют исходный `PACKET_ID`.

## Команды протокола

| Команда | Код | Назначение | Payload запроса | Payload ответа |
| --- | ---: | --- | --- | --- |
| `CMD_GO_TO_BOOT_MODE` | `0x77` | Переход в boot mode | Зависит от устройства | В текущей реализации ответ не отправляется |
| `CMD_DEV_INFO` | `0x80` | Запрос информации об устройстве | Определяется протоколом | Не реализована в текущем API |
| `CMD_PING` | `0x81` | Проверка доступности Slave | Пустой | `LAST_ERROR` |
| `CMD_HW_RESET` | `0x82` | Аппаратный сброс | Пустой | `LAST_ERROR` |
| `CMD_SW_RESET` | `0x83` | Программный сброс | Пустой | `LAST_ERROR` |
| `CMD_GET_ERROR` | `0x84` | Чтение записи журнала ошибок | `ERROR_POSITION` | `ERROR_POSITION`, `ERROR_VALUE`, `LAST_ERROR` |
| `CMD_ZEROIZE_ERROR` | `0x85` | Очистка ошибок | Пустой | `LAST_ERROR` |
| `CMD_SET_PARAM` | `0x86` | Установка параметра | Raw application-defined bytes | Raw application-defined bytes |
| `CMD_GET_PARAM` | `0x87` | Чтение параметра | Raw application-defined bytes | Raw application-defined bytes |
| `CMD_ASYNC_DATA_TRANSFER` | `0x8B` | Асинхронная передача | Данные | Не поддерживается server API |
| `CMD_UNKNOWN` | `0x00` | Ответ на неизвестную команду | Не применяется | `LAST_ERROR` |

В публичном заголовке команды доступны как `SRVINT_FC_*`. 
Команды `DEV_INFO` и `ASYNC_DATA_TRANSFER` описаны в исходной спецификации, но не входят в первую реализацию текущего API.

## Client API

Клиент создаёт serial-контекст, настраивает адрес и соединение, после чего вызывает команды высокого уровня или универсальную raw-функцию:

```c
#include <srvint.h>

srvint_t *ctx = srvint_serial_new("/dev/ttyUSB0", 115200, 'N', 8, 1);
if (ctx == NULL) {
    return EXIT_FAILURE;
}

srvint_set_slave(ctx, SRVINT_DEVICE_ADDRESS);
srvint_connect(ctx);

uint8_t last_error = 0;
if (srvint_ping(ctx, &last_error) != 0) {
    /* errno содержит причину ошибки. */
}

srvint_close(ctx);
srvint_free(ctx);
```

Основные функции клиента:

- `srvint_ping()`;
- `srvint_hw_reset()`;
- `srvint_sw_reset()`;
- `srvint_get_error()`;
- `srvint_zeroize_error()`;
- `srvint_unknown()`;
- `srvint_get_param()` и `srvint_set_param()`;
- `srvint_request()` для произвольной команды и raw buffers.

## Server API

Контекст `srvint_t` используется и для серверского режима. 
Библиотека принимает кадр, проверяет его структуру и checksum, а приложение управляет циклом обработки:

```c
static int handle_params(
    srvint_t *ctx,
    const uint8_t *request,
    size_t request_length,
    uint8_t *response,
    size_t response_capacity,
    size_t *response_length,
    void *user_data) {
    (void)ctx;
    (void)user_data;

    /* request[0..5] - header, request[6..] - raw operands. */
    if (request_length < 8 || request[4] < 2 || response_capacity < 1) {
        return -1;
    }

    /* Например: request[6] = group, request[7] = parameter. */
    response[0] = 0;
    *response_length = 1;
    return 0;
}

srvint_t *ctx = srvint_serial_new("/dev/ttyUSB0", 115200, 'N', 8, 1);
srvint_set_debug(ctx, TRUE);
srvint_set_slave(ctx, SRVINT_DEVICE_ADDRESS);
srvint_connect(ctx);

uint8_t request[6 + 255 + 1];
for (;;) {
    int length = srvint_receive(ctx, request, sizeof(request));
    if (length < 0) {
        continue;
    }
    if (srvint_reply(ctx, request, (size_t)length, handle_params, NULL) < 0) {
        break;
    }
}

srvint_close(ctx);
srvint_free(ctx);
```

Callback получает полный validated frame, а не только payload. 
Заголовок имеет следующие offsets:

```text
request[0] = START_BYTE
request[1] = ADR
request[2] = PACKET_ID
request[3] = CMD
request[4] = DATA_COUNT
request[5] = HASH
request[6...] = raw operands
```

Для `GET_PARAM` и `SET_PARAM` библиотека не знает структуру operands. 
Она не хранит parameter mapping, не выбирает endian order и не декодирует типы.

Встроенные server handlers автоматически обрабатывают `PING`, `HW_RESET`, `SW_RESET`, `GET_ERROR`, `ZEROIZE_ERROR` и неизвестные команды. 
Состояние ошибок управляется через:

- `srvint_set_last_error()`;
- `srvint_set_error()`;
- `srvint_zeroize_errors()`.

Для broadcast callback может быть вызван, но сформированный ответ всегда отбрасывается.

## Диагностическое логирование

До подключения можно включить debug logging:

```c
srvint_set_debug(ctx, TRUE);
```

В `stdout` выводятся полные RX/TX кадры:

```text
[SRV TX] 55 10 01 81 00 90
[SRV RX] 55 ca 01 81 01 4b 00 00
```

## Лицензия

См. файл `LICENSE` в репозитории.
