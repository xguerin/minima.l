include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-const-variable")

LEMON(parser SYNTAX_SOURCE)
RAGEL(lexer ${SYNTAX_SOURCE} SOURCES)

add_library(minimal SHARED ${SOURCES})
set_property(TARGET minimal PROPERTY C_STANDARD 99)
target_link_libraries(minimal PUBLIC ${CMAKE_DL_LIBS})

if(MNML_HAS_IPO)
  set_property(TARGET minimal PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS minimal LIBRARY DESTINATION lib)
