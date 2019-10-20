file(GLOB LTESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.l)
list(REMOVE_ITEM LTESTS test.l)

foreach(LTEST ${LTESTS})
  get_filename_component(TAG ${LTEST} NAME_WE)
  add_test(NAME minimal_test_${TAG} COMMAND mnml ${CMAKE_CURRENT_SOURCE_DIR}/${LTEST})
  set_property(TEST minimal_test_${TAG} PROPERTY ENVIRONMENT MNML_PLUGIN_PATH=${CMAKE_BINARY_DIR}/lib/c)
endforeach()
