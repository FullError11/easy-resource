cmake_minimum_required(VERSION 3.20)
project(erc-compiler LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/erc_debug.cmake)

erc_debug("[erc-compiler] BUILD_TYPE='${CMAKE_BUILD_TYPE}'")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(erc-compiler
    src/main.cpp
)

# 固定输出到 bin/，方便上游定位
set_target_properties(erc-compiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
foreach(cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(erc-compiler PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${cfg} "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

# Termux/NDK 下防止找不到 libc++_shared.so
if(ANDROID OR DEFINED ENV{TERMUX_VERSION})
  target_link_options(erc-compiler PRIVATE -static-libstdc++)
endif()
