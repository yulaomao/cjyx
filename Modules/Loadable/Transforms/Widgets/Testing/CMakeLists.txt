
set(KIT ${PROJECT_NAME})
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "DEBUG_LEAKS_ENABLE_EXIT_ERROR();")
set(TEST_SOURCES
  qDMMLTransformInfoWidgetTest1.cxx
  qDMMLTransformDisplayNodeWidgetTest1.cxx
  EXTRA_INCLUDE vtkDMMLDebugLeaksMacro.h
  )

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT} )
set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Module-${MODULE_NAME}")

simple_test( qDMMLTransformInfoWidgetTest1 )
simple_test( qDMMLTransformDisplayNodeWidgetTest1 )
