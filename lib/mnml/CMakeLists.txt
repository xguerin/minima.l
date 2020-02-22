include_directories(${CMAKE_SOURCE_DIR})

#
# Native plugins.
#

file(GLOB C_SOURCES *.c)

foreach(SOURCE ${C_SOURCES})
  get_filename_component(TAG ${SOURCE} NAME_WE)
  add_library(minimal_function_${TAG} SHARED ${SOURCE})
  target_link_libraries(minimal_function_${TAG} PUBLIC minimal)
  set_property(TARGET minimal_function_${TAG} PROPERTY C_STANDARD 99)
  list(APPEND PLUGINS minimal_function_${TAG})
  #
  if(MNML_HAS_IPO)
    set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
  #
  install(TARGETS minimal_function_${TAG} LIBRARY DESTINATION lib/mnml)
endforeach()

add_custom_target(minimal_plugins DEPENDS ${PLUGINS})

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
    add_library(minimal_function_${TAG} SHARED ${TARGET})
    target_link_libraries(minimal_function_${TAG} PUBLIC minimal)
    set_property(TARGET minimal_function_${TAG} PROPERTY C_STANDARD 99)
    #
    if(MNML_HAS_IPO)
      set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    #
    install(TARGETS minimal_function_${TAG} LIBRARY DESTINATION lib/mnml)
  endforeach()
endif()
