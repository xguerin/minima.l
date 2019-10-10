include_directories(${CMAKE_SOURCE_DIR})

file(GLOB C_SOURCES *.c)

foreach(SOURCE ${C_SOURCES})
  get_filename_component(TAG ${SOURCE} NAME_WE)
  add_library(minimal_function_${TAG} SHARED ${SOURCE})
  target_link_libraries(minimal_function_${TAG} PUBLIC minimal)
  set_property(TARGET minimal_function_${TAG} PROPERTY C_STANDARD 99)
  #
  if(MNML_HAS_IPO)
    set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
  #
  install(TARGETS minimal_function_${TAG} LIBRARY DESTINATION lib)
endforeach()

file(GLOB CXX_SOURCES *.cpp)

foreach(SOURCE ${CXX_SOURCES})
  get_filename_component(TAG ${SOURCE} NAME_WE)
  #
  if("${TAG}" STREQUAL "compile")
    include_directories(${LLVM_INCLUDE_DIRS})
    link_directories(${LLVM_LIBRARY_DIRS})
    add_library(minimal_function_${TAG} SHARED ${SOURCE})
    target_link_libraries(minimal_function_${TAG} PUBLIC minimal ${LLVM_LIBRARIES})
  else()
    add_library(minimal_function_${TAG} SHARED ${SOURCE})
    target_link_libraries(minimal_function_${TAG} PUBLIC minimal)
  endif()
    set_property(TARGET minimal_function_${TAG} PROPERTY CXX_STANDARD 11)
  #
  if(MNML_HAS_IPO)
    set_property(TARGET minimal_function_${TAG} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
  #
  install(TARGETS minimal_function_${TAG} LIBRARY DESTINATION lib)
endforeach()
