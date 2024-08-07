add_subdirectory(core)
add_subdirectory(grammar)
add_subdirectory(stubs)

if(MNML_WITH_MODULES)
  add_subdirectory(mnml)
endif()

add_library(minimal STATIC
  $<TARGET_OBJECTS:minimal_core>
  $<TARGET_OBJECTS:minimal_grammar>)

add_library(minimal_dyn SHARED
  $<TARGET_OBJECTS:minimal_core>
  $<TARGET_OBJECTS:minimal_grammar>
  $<TARGET_OBJECTS:minimal_stubs>)

set_target_properties(minimal_dyn PROPERTIES OUTPUT_NAME minimal)
install(TARGETS minimal_dyn LIBRARY)
