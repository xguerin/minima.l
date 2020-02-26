file(GLOB LTESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.l)
list(REMOVE_ITEM LTESTS test.l)

foreach(LTEST ${LTESTS})
  get_filename_component(TAG ${LTEST} NAME_WE)
  add_test(NAME minimal_test_${TAG} COMMAND mnml ${CMAKE_CURRENT_SOURCE_DIR}/${LTEST})
  set_property(TEST minimal_test_${TAG}
    APPEND
    PROPERTY
    ENVIRONMENT MNML_SCRIPT_PATH=${CMAKE_SOURCE_DIR}/share/mnml)
endforeach()
