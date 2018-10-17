include_directories(${CMAKE_SOURCE_DIR})
set(TESTS syntax)

foreach(TEST ${TESTS})
  add_executable(minimal_test_${TEST} ${TEST}.c)
  target_link_libraries(minimal_test_${TEST} minimal_lisp $<TARGET_OBJECTS:minimal_syntax>)
  set_property(TARGET minimal_test_${TEST} PROPERTY C_STANDARD 99)
  add_test(NAME minimal_test_${TEST} COMMAND minimal_test_${TEST})
endforeach()


