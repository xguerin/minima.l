include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)
add_library(minimal_math OBJECT ${SOURCES})
set_property(TARGET minimal_math PROPERTY C_STANDARD 99)
