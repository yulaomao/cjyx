set(KIT ${PROJECT_NAME})

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkDMMLCameraDisplayableManagerTest1.cxx
  vtkDMMLCameraWidgetTest1.cxx
  vtkDMMLModelDisplayableManagerTest.cxx
  vtkDMMLModelSliceDisplayableManagerTest.cxx
  vtkDMMLThreeDReformatDisplayableManagerTest1.cxx
  vtkDMMLThreeDViewDisplayableManagerFactoryTest1.cxx
  vtkDMMLDisplayableManagerFactoriesTest1.cxx
  vtkDMMLSliceViewDisplayableManagerFactoryTest.cxx
  EXTRA_INCLUDE vtkDMMLDebugLeaksMacro.h
  )

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

set(DisplayableManagerTest_SRCS
  vtkDMMLTestThreeDViewDisplayableManager.cxx
  vtkDMMLTestSliceViewDisplayableManager.cxx
  vtkDMMLTestCustomDisplayableManager.cxx
  )

# Create the instantiator for DisplayableManager classes.
CjyxConfigureDisplayableManagerObjectFactory(
  TARGET_NAME ${KIT}CxxTests
  SRCS "${DisplayableManagerTest_SRCS}"
  NO_EXPORT
  OUTPUT_SRCS_VAR DisplayableManagerInstantiatorTest_SRCS
  )

ctk_add_executable_utf8(${KIT}CxxTests
  ${Tests}
  ${DisplayableManagerTest_SRCS}
  ${DisplayableManagerInstantiatorTest_SRCS})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#
# Add all the tests
#
set(TEMP "${Cjyx_BINARY_DIR}/Testing/Temporary")
foreach(test ${TestsToRun})
  get_filename_component(TESTNAME ${test} NAME_WE)
  simple_test(
    ${TESTNAME}
      -D ${CMAKE_CURRENT_SOURCE_DIR}/../
      -T ${TEMP}
      -V Baseline/${TESTNAME}.png
    )
endforeach()

set_tests_properties(vtkDMMLCameraDisplayableManagerTest1 PROPERTIES RUN_SERIAL TRUE)
