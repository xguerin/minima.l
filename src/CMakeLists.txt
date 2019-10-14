add_subdirectory(compiler)
add_subdirectory(core)
add_subdirectory(grammar)

add_library(minimal SHARED
  $<TARGET_OBJECTS:minimal_compiler>
  $<TARGET_OBJECTS:minimal_core>
  $<TARGET_OBJECTS:minimal_grammar>)

set_property(TARGET minimal PROPERTY C_STANDARD 99)
target_link_libraries(minimal PUBLIC ${CMAKE_DL_LIBS})

if(MNML_HAS_IPO)
  set_property(TARGET minimal PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS minimal LIBRARY DESTINATION lib)
