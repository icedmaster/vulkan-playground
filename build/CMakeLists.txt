set (PROJECT vulkan-playground)
project (${PROJECT})

cmake_minimum_required (VERSION 2.8)

set(ROOT_DIR ${CMAKE_SOURCE_DIR}/..)
set(SRC_DIR ${ROOT_DIR}/src)
set(THIRD_PARTY_DIR ${ROOT_DIR}/3rdparty)

add_definitions(-std=c++11)
add_definitions(-m64)

include_directories(${THIRD_PARTY_DIR})
include_directories(${SRC_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

file(GLOB SOURCE ${SRC_DIR}/*.cpp)
file(GLOB HEADERS ${SRC_DIR}/*.hpp)

find_library(VULKAN_LIB vulkan-1 ${THIRD_PARTY_DIR}/vulkan)
message(${VULKAN_LIB})
set(LIBS ${VULKAN_LIB})

find_library(ASSIMP_LIB assimp ${THIRD_PARTY_DIR}/assimp)
message(${ASSIMP_LIB})
set(LIBS ${LIBS} ${ASSIMP_LIB})

add_subdirectory(${CMAKE_SOURCE_DIR}/../samples/00_cube/build/ ${CMAKE_SOURCE_DIR}/../output/00_cube)
add_subdirectory(${CMAKE_SOURCE_DIR}/../samples/01_deferred/build/ ${CMAKE_SOURCE_DIR}/../output/01_deferred)
add_subdirectory(${CMAKE_SOURCE_DIR}/../samples/02_sponza/build/ ${CMAKE_SOURCE_DIR}/../output/02_sponza)

