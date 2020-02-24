file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)

add_library(minimal_module OBJECT ${SOURCES})
set_property(TARGET minimal_module PROPERTY C_STANDARD 99)
