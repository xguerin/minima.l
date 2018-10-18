include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(minimal_functions OBJECT ${SOURCES})
set_property(TARGET minimal_functions PROPERTY C_STANDARD 99)
