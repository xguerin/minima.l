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
  #
  if(APPLE)
    target_link_options(${MODULE} PRIVATE
      -Wl,-U,_MNML_DEBUG
      -Wl,-U,_MNML_DEBUG_CLOS
      -Wl,-U,_MNML_DEBUG_EVAL
      -Wl,-U,_lisp_bind
      -Wl,-U,_lisp_car
      -Wl,-U,_lisp_cdr
      -Wl,-U,_lisp_conc
      -Wl,-U,_lisp_cons
      -Wl,-U,_lisp_deallocate
      -Wl,-U,_lisp_debug
      -Wl,-U,_lisp_decref
      -Wl,-U,_lisp_dup
      -Wl,-U,_lisp_equ
      -Wl,-U,_lisp_eval
      -Wl,-U,_lisp_get_fullpath
      -Wl,-U,_lisp_incref
      -Wl,-U,_lisp_is_string
      -Wl,-U,_lisp_len
      -Wl,-U,_lisp_load_file
      -Wl,-U,_lisp_make_char
      -Wl,-U,_lisp_make_cstring
      -Wl,-U,_lisp_make_nil
      -Wl,-U,_lisp_make_number
      -Wl,-U,_lisp_make_quote
      -Wl,-U,_lisp_make_string
      -Wl,-U,_lisp_make_symbol
      -Wl,-U,_lisp_make_true
      -Wl,-U,_lisp_mark_tail_calls
      -Wl,-U,_lisp_merge
      -Wl,-U,_lisp_neq
      -Wl,-U,_lisp_prin
      -Wl,-U,_lisp_prog
      -Wl,-U,_lisp_read
      -Wl,-U,_lisp_setq
      -Wl,-U,_lisp_timestamp
      -Wl,-U,_lisp_tree_upd
      -Wl,-U,_module_load)
  endif()
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
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
  #
  Transpile(iterators "foldl,foldr,map,map2,zip")
  Transpile(manips "assoc,caar,cddr,cadar,caddr,cadr,cdar,last,rep,truncl")
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
