include_directories(${CMAKE_SOURCE_DIR})

file(GLOB C_SOURCES *.c)

foreach(SOURCE ${C_SOURCES})
  get_filename_component(TAG ${SOURCE} NAME_WE)
  add_library(minimal_function_${TAG} SHARED ${SOURCE})
  target_link_libraries(minimal_function_${TAG} PUBLIC minimal)
  set_property(TARGET minimal_function_${TAG} PROPERTY C_STANDARD 99)
  list(APPEND PLUGINS minimal_function_${TAG})
  #
  if(MNML_HAS_IPO)
    set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
  #
  install(TARGETS minimal_function_${TAG} LIBRARY DESTINATION lib)
endforeach()

add_custom_target(minimal_plugins DEPENDS ${PLUGINS})
