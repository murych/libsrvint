if(NOT UNIX)
  message(
    STATUS "Skipping pseudo-terminal integration test on non-UNIX platform")
  return()
endif()

find_library(UTIL_LIBRARY util REQUIRED)

add_executable(libsrvint_pty_request_test pty_request.c)
target_link_libraries(libsrvint_pty_request_test PRIVATE libsrvint
                                                         ${UTIL_LIBRARY})
target_compile_features(libsrvint_pty_request_test PRIVATE c_std_99)

add_test(NAME libsrvint_pty_request COMMAND libsrvint_pty_request_test)

add_executable(libsrvint_basic_commands_test basic_commands.c)
target_link_libraries(libsrvint_basic_commands_test PRIVATE libsrvint
                                                            ${UTIL_LIBRARY})
target_compile_features(libsrvint_basic_commands_test PRIVATE c_std_99)

add_test(NAME libsrvint_basic_commands COMMAND libsrvint_basic_commands_test)

add_executable(libsrvint_server_loop_test server_loop.c)
target_link_libraries(libsrvint_server_loop_test PRIVATE libsrvint
                                                         ${UTIL_LIBRARY})
target_compile_features(libsrvint_server_loop_test PRIVATE c_std_99)

add_test(NAME libsrvint_server_loop COMMAND libsrvint_server_loop_test)
