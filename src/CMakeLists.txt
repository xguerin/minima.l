file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_library(minimal SHARED ${SOURCES})
set_property(TARGET minimal PROPERTY C_STANDARD 99)
target_link_libraries(minimal PRIVATE minimal_grammar PUBLIC ${CMAKE_DL_LIBS})

if(MNML_HAS_IPO)
  set_property(TARGET minimal PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

install(TARGETS minimal LIBRARY DESTINATION lib)
