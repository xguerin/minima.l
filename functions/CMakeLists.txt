include_directories(${CMAKE_SOURCE_DIR})

file(GLOB SOURCES *.c)

foreach(SOURCE ${SOURCES})
  get_filename_component(TAG ${SOURCE} NAME_WE)
  add_library(minimal_function_${TAG} SHARED ${SOURCE})
  target_link_libraries(minimal_function_${TAG} minimal)
  set_property(TARGET minimal_function_${TAG} PROPERTY C_STANDARD 99)
  #
  if(MNML_HAS_IPO)
    set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endforeach()
