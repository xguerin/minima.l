add_subdirectory(io)
add_subdirectory(logic)
add_subdirectory(math)
add_subdirectory(std)
add_subdirectory(sys)
add_subdirectory(unix)

#
# Native modules.
#

set(MODULES io logic math std sys unix)

foreach(MODULE ${MODULES})
  add_library(${MODULE} SHARED $<TARGET_OBJECTS:minimal_${MODULE}>)
  add_dependencies(${MODULE} minimal)
  target_link_libraries(${MODULE} PUBLIC minimal)
  #
  if(MNML_HAS_IPO)
    set_property(TARGET ${MODULE} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
  #
  install(TARGETS ${MODULE} LIBRARY DESTINATION lib/mnml)
endforeach()

add_custom_target(minimal_modules DEPENDS ${MODULES})

#
# Transpiled plugins.
#

macro(Transpile TAG SYMBOLS)
  add_custom_command(
    DEPENDS
    ${CMAKE_SOURCE_DIR}/bin/mnmlc
    ${CMAKE_SOURCE_DIR}/share/mnml/cc.l
    ${CMAKE_SOURCE_DIR}/share/mnml/cps.l
    COMMAND
    MNML_SCRIPT_PATH=${CMAKE_SOURCE_DIR}/share/mnml
    ${CMAKE_BINARY_DIR}/bin/mnml ${CMAKE_SOURCE_DIR}/bin/mnmlc
    -c ${CMAKE_SOURCE_DIR}/share/mnml/${TAG}.l -m ${TAG} -s ${SYMBOLS} -o ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c
    MAIN_DEPENDENCY ${SCRIPT}
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c
    DEPENDS mnml minimal_modules)
endmacro()

set(MODULES iterators manips)

if(MNML_WITH_TRANSFN)
  #
  Transpile(iterators "foldl,foldr,map,map2")
  Transpile(manips "assoc,caar,cadar,caddr,cadr,cdar,last,rep,truncl")
  #
  foreach(MODULE ${MODULES})
    add_library(${MODULE} SHARED ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.c)
    target_link_libraries(${MODULE} PUBLIC minimal)
    set_property(TARGET ${MODULE} PROPERTY C_STANDARD 99)
    #
    if(MNML_HAS_IPO)
      set_property(TARGET ${MODULE} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    #
    install(TARGETS ${MODULE} LIBRARY DESTINATION lib/mnml)
  endforeach()
endif()
