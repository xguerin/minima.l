include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-const-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-compare")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-void-pointer-to-int-cast")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-to-int-cast")
endif()

LEMON(parser SYNTAX_SOURCE)
RAGEL(lexer ${SYNTAX_SOURCE} SOURCES)

add_library(minimal_grammar OBJECT ${SOURCES})
set_property(TARGET minimal_grammar PROPERTY C_STANDARD 99)
