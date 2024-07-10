cmake_minimum_required(VERSION 3.10)
project(KV-WorkloadGenerator VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/load_gen.cc 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Generator.cc 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Key.cc
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(load_gen ${SOURCES})