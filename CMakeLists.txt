cmake_minimum_required(VERSION 3.14)

project(easy_resource)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(erc INTERFACE
        include/erc/erc.h
)
target_include_directories(erc INTERFACE
        include
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/erc.cmake)
