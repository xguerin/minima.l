include_directories(${CMAKE_SOURCE_DIR})

file(GLOB TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c)
list(REMOVE_ITEM TESTS utils.c)

foreach(TEST ${TESTS})
  get_filename_component(TAG ${TEST} NAME_WE)
  add_executable(minimal_test_${TAG} ${TEST} utils.c)
  set_property(TARGET minimal_test_${TAG} PROPERTY C_STANDARD 99)
  add_test(NAME minimal_test_${TAG} COMMAND minimal_test_${TAG})
  target_link_libraries(minimal_test_${TAG} minimal)
endforeach()


