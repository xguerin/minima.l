add_subdirectory(core)
add_subdirectory(grammar)

if(MNML_WITH_MODULES)
  add_subdirectory(mnml)
endif()

add_library(minimal STATIC
  $<TARGET_OBJECTS:minimal_core>
  $<TARGET_OBJECTS:minimal_grammar>)
