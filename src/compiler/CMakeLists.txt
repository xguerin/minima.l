include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(minimal_compiler OBJECT ${SOURCES})
set_property(TARGET minimal_compiler PROPERTY C_STANDARD 99)
