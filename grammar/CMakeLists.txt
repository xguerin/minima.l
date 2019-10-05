include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-compare")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-const-variable")

LEMON(parser SYNTAX_SOURCE)
RAGEL(lexer ${SYNTAX_SOURCE} SOURCES)

add_library(minimal_grammar STATIC ${SOURCES})
set_property(TARGET minimal_grammar PROPERTY C_STANDARD 99)

if(MNML_HAS_IPO)
  set_property(TARGET minimal_grammar PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
