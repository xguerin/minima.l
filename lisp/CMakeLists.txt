include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-const-variable")

LEMON(parser SYNTAX_SOURCE)
RAGEL(lexer ${SYNTAX_SOURCE} SOURCES)

add_library(minimal_lisp STATIC ${SOURCES})
set_property(TARGET minimal_lisp PROPERTY C_STANDARD 99)
