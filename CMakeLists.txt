cmake_minimum_required(VERSION 3.14)

project(
  libsrvint
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT libsrvint_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(AUTHOR_WARNING "Developer mode is intended for developers")
endif()

if(BUILD_TESTING)
  add_library(ref INTERFACE src/_ref/Errors.h src/_ref/SrvIntCMD_extractor.h)
endif()
