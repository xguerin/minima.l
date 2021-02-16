include_directories(${CMAKE_SOURCE_DIR})

file(GLOB CTESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
list(REMOVE_ITEM CTESTS utils.c)

foreach(CTEST ${CTESTS})
  get_filename_component(TAG ${CTEST} NAME_WE)
  add_executable(minimal_test_c_${TAG} ${CTEST} utils.c)
  set_property(TARGET minimal_test_c_${TAG} PROPERTY C_STANDARD 99)
  add_test(NAME minimal_test_c_${TAG} COMMAND minimal_test_c_${TAG})
  target_link_libraries(minimal_test_c_${TAG} minimal)
endforeach()
