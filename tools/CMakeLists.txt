include_directories(${CMAKE_SOURCE_DIR})

add_executable(mnml mnml.c)
target_link_libraries(mnml minimal_lisp $<TARGET_OBJECTS:minimal_syntax>)
set_property(TARGET mnml PROPERTY C_STANDARD 99)
