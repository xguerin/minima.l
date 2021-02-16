add_subdirectory(c)
add_subdirectory(unit)

if(MNML_ENABLE_LISPY_TESTS)
  add_subdirectory(lisp)
endif()
