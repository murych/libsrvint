if(NOT UNIX)
  message(STATUS "Skipping pseudo-terminal integration test on non-UNIX platform")
  return()
endif()

find_library(UTIL_LIBRARY util)
if(NOT UTIL_LIBRARY)
  message(FATAL_ERROR "libutil is required for the pseudo-terminal integration test")
endif()

add_executable(libsrvint_pty_request_test pty_request.c)
target_link_libraries(libsrvint_pty_request_test PRIVATE libsrvint ${UTIL_LIBRARY})
target_compile_features(libsrvint_pty_request_test PRIVATE c_std_99)

add_test(NAME libsrvint_pty_request COMMAND libsrvint_pty_request_test)
