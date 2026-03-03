cmake_minimum_required(VERSION 3.14)
project(openDAQ_LT_Tests LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# For Windows: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# mjson (fetched since not in tree)
FetchContent_Declare(
  mjson
  URL https://github.com/cesanta/mjson/archive/refs/tags/1.2.7.tar.gz
)
FetchContent_MakeAvailable(mjson)

# mpack (fetched since not in tree)
FetchContent_Declare(
  mpack
  URL https://github.com/ludocode/mpack/archive/refs/tags/v1.1.tar.gz
)
FetchContent_MakeAvailable(mpack)

# Add mock headers include path
include_directories(tests/mocks)

# Add source includes
include_directories(segger/streaming)
include_directories(segger/discovery)

# Mock libs include paths to match existing code's #include "libs/mjson/src/mjson.h"
# and #include "mjson/src/mjson.h"
# We'll need to create symlinks or dummy directories if we want to avoid modifying source code.
# Alternatively, we can use a wrapper header.
# Let's try to map the fetched mjson correctly.
set(MJSON_DIR ${mjson_SOURCE_DIR})
include_directories(${MJSON_DIR})
# Create a dummy "libs/mjson/src" directory structure to satisfy #include "libs/mjson/src/mjson.h"
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/dummy_includes/libs/mjson/src)
file(CREATE_LINK ${MJSON_DIR}/mjson.h ${CMAKE_BINARY_DIR}/dummy_includes/libs/mjson/src/mjson.h SYMBOLIC)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/dummy_includes/mjson/src)
file(CREATE_LINK ${MJSON_DIR}/mjson.h ${CMAKE_BINARY_DIR}/dummy_includes/mjson/src/mjson.h SYMBOLIC)
include_directories(${CMAKE_BINARY_DIR}/dummy_includes)

# mpack.h is usually included directly
include_directories(${mpack_SOURCE_DIR}/src/mpack)

enable_testing()

# We will add tests in the next phases.
# For now, just have a placeholder test to verify build.
add_executable(unit_tests
  tests/unit/stream_id_test.cpp
  tests/unit/streaming_signals_test.cpp
  tests/mocks/mocks.cpp
  segger/streaming/stream_id.c
  segger/streaming/streaming_signals.c
)
target_link_libraries(unit_tests GTest::gtest_main)
add_test(NAME placeholder_test COMMAND unit_tests)
