cmake_minimum_required(VERSION 3.14)

project(
  libsrvint
  VERSION 0.1
  LANGUAGES C CXX)

set(SRVINT_START_BYTE "0x55" CACHE STRING
    "SrvInt frame start byte (the protocol document's 0xB7 is not the default)")

option(LIBSRVINT_BUILD_TESTS "Build libsrvint integration tests" OFF)
option(LIBSRVINT_BUILD_EXAMPLES "Build libsrvint usage examples" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(libsrvint)

include(GenerateExportHeader)
generate_export_header(
  libsrvint
  BASE_NAME
  libsrvint
  EXPORT_MACRO_NAME
  SRVINT_API
  EXPORT_FILE_NAME
  export/libsrvint/libsrvint_export.h)

target_sources(
  libsrvint
  PRIVATE src/srvint.c src/srvint_private.h
  PUBLIC FILE_SET
         HEADERS
         FILES
         src/srvint.h
         FILE_SET
         generated_headers
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}/export/libsrvint/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/export/libsrvint/libsrvint_export.h)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(libsrvint PUBLIC LIBSRVINT_STATIC_DEFINE)
endif()
target_compile_definitions(libsrvint
                           PUBLIC SRVINT_START_BYTE=${SRVINT_START_BYTE})

set_target_properties(
  libsrvint
  PROPERTIES C_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES
             VERSION "${PROJECT_VERSION}"
             SOVERSION "${PROJECT_VERSION_MAJOR}"
             EXPORT_NAME srvint
             OUTPUT_NAME srvint
             PUBLIC_HEADER src/srvint.h)

target_include_directories(
  libsrvint
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  SYSTEM $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export/libsrvint>)
target_compile_features(libsrvint PUBLIC c_std_99)

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(GNUInstallDirs)
  install(
    TARGETS libsrvint
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(NOT libsrvint_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(AUTHOR_WARNING "Developer mode is intended for developers")
endif()

if(LIBSRVINT_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif()

if(LIBSRVINT_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
