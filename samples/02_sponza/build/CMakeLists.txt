set (PROJECT 02_sponza)
project (${PROJECT})

cmake_minimum_required (VERSION 2.8)

add_definitions(-std=c++11)

include_directories(${THIRD_PARTY_DIR})
include_directories(${SRC_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../bin)

file(GLOB SAMPLE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../*.cpp)
file(GLOB SAMPLE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../*.hpp)

add_executable(${PROJECT} ${SOURCE} ${HEADERS} ${SAMPLE_SOURCES} ${SAMPLE_HEADERS})
target_link_libraries(${PROJECT} ${LIBS})