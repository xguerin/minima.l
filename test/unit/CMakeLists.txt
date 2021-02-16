file(GLOB LTESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.l)

foreach(LTEST ${LTESTS})
  get_filename_component(TAG ${LTEST} NAME_WE)
  add_test(NAME minimal_test_unit_${TAG} COMMAND mnml -d ${CMAKE_CURRENT_SOURCE_DIR}/${LTEST})
  set_property(TEST minimal_test_unit_${TAG}
    APPEND
    PROPERTY
    ENVIRONMENT MNML_SCRIPT_PATH=${CMAKE_SOURCE_DIR}/share/mnml)
endforeach()
