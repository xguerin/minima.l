include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-const-variable")

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

LEMON(parser SYNTAX_SOURCE)
RAGEL(lexer ${SYNTAX_SOURCE} SOURCES)

add_library(minimal_syntax OBJECT ${SOURCES})
set_property(TARGET minimal_syntax PROPERTY C_STANDARD 99)
