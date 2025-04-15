cmake_minimum_required(VERSION 3.8)

project(libintelpmt VERSION 0.1.0)

execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_pmt_headers.py ${CMAKE_CURRENT_SOURCE_DIR}/lib/Intel-PMT ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB GENERATED_FILES ${CMAKE_CURRENT_BINARY_DIR}/src/*)

add_library(libintelpmt  ${GENERATED_FILES})

target_include_directories(libintelpmt PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include include)

add_executable(pmttool examples/pmttool.cpp)
target_link_libraries(pmttool libintelpmt)
