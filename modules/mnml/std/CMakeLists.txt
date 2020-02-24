include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)
add_library(minimal_std OBJECT ${SOURCES})
set_property(TARGET minimal_std PROPERTY C_STANDARD 99)
