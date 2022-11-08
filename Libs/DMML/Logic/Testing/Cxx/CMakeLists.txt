set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(${KIT}Testing_ITK_COMPONENTS
  ITKCommon
  )
find_package(ITK 4.6 COMPONENTS ${${KIT}Testing_ITK_COMPONENTS} REQUIRED)
if(ITK_VERSION VERSION_GREATER_EQUAL "5.3")
  foreach(factory_uc IN ITEMS "IMAGEIO" "MESHIO" "TRANSFORMIO")
    set(ITK_NO_${factory_uc}_FACTORY_REGISTER_MANAGER 1)
  endforeach()
else()
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
endif()
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(EXTRA_INCLUDE "vtkDMMLDebugLeaksMacro.h\"\n\#include \"vtkTestingOutputWindow.h")
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();\nTESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDMMLAbstractLogicSceneEventsTest.cxx
  vtkDMMLColorLogicTest1.cxx
  vtkDMMLDisplayableHierarchyLogicTest1.cxx
  vtkDMMLLayoutLogicCompareTest.cxx
  vtkDMMLLayoutLogicTest1.cxx
  vtkDMMLLayoutLogicTest2.cxx
  vtkDMMLSliceLayerLogicTest.cxx
  vtkDMMLSliceLogicTest1.cxx
  vtkDMMLSliceLogicTest2.cxx
  vtkDMMLSliceLogicTest3.cxx
  vtkDMMLSliceLogicTest4.cxx
  vtkDMMLSliceLogicTest5.cxx
  vtkDMMLApplicationLogicTest1.cxx
  EXTRA_INCLUDE ${EXTRA_INCLUDE}
  )

ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT} ${ITK_LIBRARIES})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#-----------------------------------------------------------------------------
macro(simple_file_test TESTNAME FILE)
  simple_test(${TESTNAME}_${FILE} DRIVER_TESTNAME ${TESTNAME} DATA{${DMMLCore_SOURCE_DIR}/Testing/TestData/${FILE}})
endmacro()

#-----------------------------------------------------------------------------
simple_test( vtkDMMLAbstractLogicSceneEventsTest )
simple_test( vtkDMMLColorLogicTest1 )
simple_test( vtkDMMLDisplayableHierarchyLogicTest1 )
simple_test( vtkDMMLLayoutLogicCompareTest )
simple_test( vtkDMMLLayoutLogicTest1 )
simple_test( vtkDMMLLayoutLogicTest2 )
simple_test( vtkDMMLSliceLayerLogicTest )
simple_test( vtkDMMLSliceLogicTest1 )
simple_file_test( vtkDMMLSliceLogicTest2 fixed.nrrd)
simple_file_test( vtkDMMLSliceLogicTest3 fixed.nrrd)
simple_file_test( vtkDMMLSliceLogicTest4 fixed.nrrd)
simple_file_test( vtkDMMLSliceLogicTest5 fixed.nrrd)
simple_test( vtkDMMLApplicationLogicTest1 "${CMAKE_BINARY_DIR}/Testing/Temporary" )
