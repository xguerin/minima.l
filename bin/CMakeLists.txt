include_directories(${CMAKE_SOURCE_DIR})

add_executable(mnml mnml.c)
target_link_libraries(mnml minimal)

set_property(TARGET mnml PROPERTY C_STANDARD 99)

if(MNML_HAS_IPO)
  set_property(TARGET mnml PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS mnml RUNTIME DESTINATION bin)
