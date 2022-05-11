include_directories(${CMAKE_SOURCE_DIR})

add_executable(mnml mnml.c)

target_link_libraries(mnml
  $<TARGET_OBJECTS:minimal_core>
  $<TARGET_OBJECTS:minimal_grammar>
  ${CMAKE_DL_LIBS})
target_link_options(mnml PRIVATE -rdynamic)

set_property(TARGET mnml PROPERTY C_STANDARD 99)

if(MNML_HAS_IPO)
  set_property(TARGET mnml PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS mnml RUNTIME DESTINATION bin)
install(PROGRAMS mnmlc DESTINATION bin)
