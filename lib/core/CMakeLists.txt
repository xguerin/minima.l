file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(minimal_core OBJECT ${SOURCES})
set_property(TARGET minimal_core PROPERTY C_STANDARD 99)
