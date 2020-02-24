include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)
add_library(minimal_unix OBJECT ${SOURCES})
set_property(TARGET minimal_unix PROPERTY C_STANDARD 99)
