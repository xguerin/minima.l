include_directories(${CMAKE_SOURCE_DIR})
set(TESTS syntax)

foreach(TEST ${TESTS})
  add_executable(minimal_test_${TEST} ${TEST}.c)
  set_property(TARGET minimal_test_${TEST} PROPERTY C_STANDARD 99)
  add_test(NAME minimal_test_${TEST} COMMAND minimal_test_${TEST})
  target_link_libraries(minimal_test_${TEST}
    $<TARGET_OBJECTS:minimal_functions>
    $<TARGET_OBJECTS:minimal_lisp>
    $<TARGET_OBJECTS:minimal_syntax>)
endforeach()


