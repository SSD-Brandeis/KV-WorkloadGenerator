cmake_minimum_required(VERSION 3.10)
project(KV-WorkloadGenerator VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES 
    ${CMAKE_SOURCE_DIR}/src/load_gen.cc 
    ${CMAKE_SOURCE_DIR}/src/Generator.cc 
    ${CMAKE_SOURCE_DIR}/src/Key.cc
)

add_executable(load_gen ${SOURCES})