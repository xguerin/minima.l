include_directories(${CMAKE_SOURCE_DIR})

add_executable(mnml mnml.c)
set_property(TARGET mnml PROPERTY C_STANDARD 99)
target_link_libraries(mnml minimal_lisp)
