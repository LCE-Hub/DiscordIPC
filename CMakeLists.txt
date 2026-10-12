cmake_minimum_required(VERSION 3.25)
project(discord_ipc_cpp
  VERSION 1.0.0
  DESCRIPTION "C++ library for interfacing with Discord IPC socket"
)

add_library(discord_ipc_cpp STATIC
  src/discord_ipc_client.cpp
  src/internal_ipc_types.cpp
  src/ipc_types.cpp
  src/json.cpp
  src/parser.cpp
  src/socket_client.cpp
  src/utils.cpp
)

set_target_properties(discord_ipc_cpp PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

set_target_properties(discord_ipc_cpp PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
)

target_include_directories(discord_ipc_cpp
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
  target_compile_options(discord_ipc_cpp PRIVATE
    /W4
    /O2
  )
else()
  target_compile_options(discord_ipc_cpp PRIVATE
    -Wall
    -Wextra
    -O3
    -pthread
    -flto
    -fvisibility=hidden
  )
endif()
