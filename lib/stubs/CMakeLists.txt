file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(minimal_stubs OBJECT ${SOURCES})
set_property(TARGET minimal_stubs PROPERTY C_STANDARD 99)
