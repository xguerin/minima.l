include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)
add_library(minimal_logic OBJECT ${SOURCES})
set_property(TARGET minimal_logic PROPERTY C_STANDARD 99)
