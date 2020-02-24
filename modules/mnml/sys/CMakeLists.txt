include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)
add_library(minimal_sys OBJECT ${SOURCES})
set_property(TARGET minimal_sys PROPERTY C_STANDARD 99)
