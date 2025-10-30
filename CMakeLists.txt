cmake_minimum_required(VERSION 3.8)

project(libintelpmt VERSION 0.1.0)


file(GLOB_RECURSE TOOL_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/*)
file(GLOB_RECURSE INTELPMT_XMLS ${CMAKE_CURRENT_SOURCE_DIR}/lib/Intel-PMT/xml/*)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/libintelpmt.cpp
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_pmt_headers.py ${CMAKE_CURRENT_SOURCE_DIR}/lib/Intel-PMT ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TOOL_FILES} ${INTELPMT_XMLS})

add_library(libintelpmt  src/libintelpmt.cpp)

target_include_directories(libintelpmt PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include include)

add_executable(pmttool examples/pmttool.cpp)
target_link_libraries(pmttool libintelpmt)
