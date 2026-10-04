cmake_minimum_required(VERSION 3.14)

project(example)

add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/easy_resource)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example
    src/main.cpp
    src/a.cpp
    src/b.cpp
    src/c.cpp
    src/d.cpp
)

erc_resource(example
    ${CMAKE_CURRENT_SOURCE_DIR}/asset
)
target_link_libraries(example PUBLIC erc)
