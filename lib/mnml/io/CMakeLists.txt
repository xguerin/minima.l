include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)
add_library(minimal_io OBJECT ${SOURCES})
set_property(TARGET minimal_io PROPERTY C_STANDARD 99)
