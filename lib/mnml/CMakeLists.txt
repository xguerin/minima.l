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

add_custom_target(minimal_plugins DEPENDS ${MODULES})

#
# Transpiled plugins.
#

macro(Transpile TAG SCRIPT TARGET)
  add_custom_command(
    COMMAND
    MNML_SCRIPT_PATH=${CMAKE_SOURCE_DIR}/lisp
    ${CMAKE_BINARY_DIR}/bin/mnml ${CMAKE_SOURCE_DIR}/bin/mnmlc -c ${SCRIPT} -o ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c
    MAIN_DEPENDENCY ${SCRIPT}
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c
    DEPENDS mnml minimal_plugins)
  set(${TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c)
endmacro()

set(SOURCES caar.l cadar.l cadr.l cdar.l foldl.l foldr.l map.l rep.l truncl.l)

if(MNML_WITH_TRANSFN)
  foreach(SOURCE ${SOURCES})
    get_filename_component(TAG ${SOURCE} NAME_WE)
    Transpile(${TAG} ${CMAKE_SOURCE_DIR}/lisp/${SOURCE} TARGET)
    add_library(${TAG} SHARED ${TARGET})
    target_link_libraries(${TAG} PUBLIC minimal)
    set_property(TARGET ${TAG} PROPERTY C_STANDARD 99)
    #
    if(MNML_HAS_IPO)
      set_property(TARGET ${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    #
    install(TARGETS ${TAG} LIBRARY DESTINATION lib/mnml)
  endforeach()
endif()
