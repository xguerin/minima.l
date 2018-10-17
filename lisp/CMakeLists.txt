file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(minimal_lisp OBJECT ${SOURCES})
set_property(TARGET minimal_lisp PROPERTY C_STANDARD 99)
