#
# Define the script list to install.
#

file(GLOB SCRIPTS *.l)
install(FILES ${SCRIPTS} DESTINATION share/mnml)

#
# Define the subset list to compile.
#

macro(Transpile TAG SCRIPT TARGET)
  add_custom_command(
    COMMAND
    MNML_SCRIPT_PATH=${CMAKE_SOURCE_DIR}/lisp
    ${CMAKE_BINARY_DIR}/bin/mnml ${CMAKE_SOURCE_DIR}/bin/mnmlc
    ${SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c
    MAIN_DEPENDENCY ${SCRIPT}
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c
    DEPENDS mnml minimal_plugins)
  set(${TARGET} ${CMAKE_CURRENT_BINARY_DIR}/${TAG}.c)
endmacro()

set(SOURCES caar.l cadar.l cadr.l cdar.l foldl.l foldr.l map.l rep.l truncl.l)

if(MNML_WITH_TRANSFN)
  foreach(SOURCE ${SOURCES})
    get_filename_component(TAG ${SOURCE} NAME_WE)
    Transpile(${TAG} ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE} TARGET)
    add_library(minimal_function_${TAG} SHARED ${TARGET})
    target_link_libraries(minimal_function_${TAG} PUBLIC minimal)
    set_property(TARGET minimal_function_${TAG} PROPERTY C_STANDARD 99)
    #
    if(MNML_HAS_IPO)
      set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
    #
    install(TARGETS minimal_function_${TAG} LIBRARY DESTINATION lib)
  endforeach()
endif()
