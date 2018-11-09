include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)

foreach(TEST ${TESTS})
  get_filename_component(TAG ${SOURCE} NAME_WE)
  add_executable(minimal_test_${TAG} ${SOURCE})
  set_property(TARGET minimal_test_${TAG} PROPERTY C_STANDARD 99)
  add_test(NAME minimal_test_${TAG} COMMAND minimal_test_${TAG})
  target_link_libraries(minimal_test_${TAG} minimal)
endforeach()


