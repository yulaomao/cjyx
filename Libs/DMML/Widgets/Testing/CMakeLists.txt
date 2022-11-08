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
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN  "
  TESTING_OUTPUT_INIT();
  itk::itkFactoryRegistration();
  ")

set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "
  TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);
  DEBUG_LEAKS_ENABLE_EXIT_ERROR();
  ")

set(TEST_SOURCES
  qDMMLCheckableNodeComboBoxTest.cxx
  qDMMLCheckableNodeComboBoxTest1.cxx
  qDMMLClipNodeWidgetTest1.cxx
  qDMMLCollapsibleButtonTest.cxx
  qDMMLColorListViewTest1.cxx
  qDMMLColorModelTest.cxx
  qDMMLColorModelTest1.cxx
  qDMMLColorPickerWidgetTest1.cxx
  qDMMLColorPickerWidgetTest2.cxx
  qDMMLColorPickerWidgetTest3.cxx
  qDMMLColorTableComboBoxTest1.cxx
  qDMMLColorTableViewTest1.cxx
  qDMMLDisplayNodeViewComboBoxTest.cxx
  qDMMLDisplayNodeWidgetTest1.cxx
  qDMMLLayoutManagerTest1.cxx
  qDMMLLayoutManagerTest2.cxx
  qDMMLLayoutManagerTest3.cxx
  qDMMLLayoutManagerTest4.cxx
  qDMMLLayoutManagerVisibilityTest.cxx
  qDMMLLayoutManagerWithCustomFactoryTest.cxx
  qDMMLLinearTransformSliderTest1.cxx
  qDMMLListWidgetTest1.cxx
  qDMMLMatrixWidgetTest1.cxx
  qDMMLModelInfoWidgetTest1.cxx
  qDMMLModelTest1.cxx
  qDMMLModelTreeViewTest1.cxx
  qDMMLLabelComboBoxTest.cxx
  qDMMLNavigationViewTest1.cxx
  qDMMLNodeAttributeTableViewTest.cxx
  qDMMLNodeAttributeTableWidgetTest.cxx
  qDMMLNodeComboBoxTest1.cxx
  qDMMLNodeComboBoxTest2.cxx
  qDMMLNodeComboBoxTest3.cxx
  qDMMLNodeComboBoxTest4.cxx
  qDMMLNodeComboBoxTest5.cxx
  qDMMLNodeComboBoxTest6.cxx
  qDMMLNodeComboBoxTest7.cxx
  qDMMLNodeComboBoxTest8.cxx
  qDMMLNodeComboBoxTest9.cxx
  qDMMLNodeComboBoxLazyUpdateTest1.cxx
  qDMMLNodeFactoryTest1.cxx
  qDMMLPlotViewTest1.cxx
  qDMMLScalarInvariantComboBoxTest1.cxx
  qDMMLSceneCategoryModelTest1.cxx
  qDMMLSceneColorTableModelTest1.cxx
  qDMMLSceneFactoryWidgetTest1.cxx
  qDMMLSceneHierarchyModelTest1.cxx
  qDMMLSceneModelTest.cxx
  qDMMLSceneModelTest1.cxx
  qDMMLSceneTransformModelTest1.cxx
  qDMMLSceneTransformModelTest2.cxx
  qDMMLSceneDisplayableModelTest1.cxx
  qDMMLSceneDisplayableModelTest2.cxx
  qDMMLSegmentSelectorWidgetTest1.cxx
  qDMMLSliceControllerWidgetTest.cxx
  qDMMLSliceWidgetTest1.cxx
  qDMMLSliceWidgetTest2.cxx
  qDMMLTableViewTest1.cxx
  qDMMLTransformSlidersTest1.cxx
  qDMMLThreeDViewTest1.cxx
  qDMMLThreeDWidgetTest1.cxx
  qDMMLTreeViewTest1.cxx
  qDMMLUtf8Test1.cxx
  qDMMLUtilsTest1.cxx
  qDMMLVolumeInfoWidgetTest1.cxx
  qDMMLVolumeThresholdWidgetTest1.cxx
  qDMMLVolumeThresholdWidgetTest2.cxx
  qDMMLWidgetsExportTest1.cxx
  qDMMLWindowLevelWidgetTest1.cxx
  EXTRA_INCLUDE qDMMLWidgetCxxTests.h
  )

if(Cjyx_USE_QtTesting)
  list(APPEND TEST_SOURCES
    qDMMLCheckableNodeComboBoxEventTranslatorPlayerTest1.cxx
    qDMMLClipNodeWidgetEventTranslatorPlayerTest1.cxx
    qDMMLCollapsibleButtonEventTranslatorPlayerTest1.cxx
    qDMMLColorListViewEventTranslatorPlayerTest1.cxx
    qDMMLColorPickerWidgetEventTranslatorPlayerTest1.cxx
    qDMMLColorTableComboBoxEventTranslatorPlayerTest1.cxx
    qDMMLColorTableViewEventTranslatorPlayerTest1.cxx
    qDMMLDisplayNodeWidgetEventTranslatorPlayerTest1.cxx
    qDMMLLabelComboBoxEventTranslatorPlayerTest1.cxx
    qDMMLLinearTransformSliderEventTranslatorPlayerTest1.cxx
    qDMMLListWidgetEventTranslatorPlayerTest1.cxx
    qDMMLMatrixWidgetEventTranslatorPlayerTest1.cxx
    qDMMLNavigationViewEventTranslatorPlayerTest1.cxx
    qDMMLNodeComboBoxEventTranslatorPlayerTest1.cxx
    qDMMLRangeWidgetEventTranslatorPlayerTest1.cxx
    qDMMLROIWidgetEventTranslatorPlayerTest1.cxx
    qDMMLScalarInvariantComboBoxEventTranslatorPlayerTest1.cxx
    qDMMLSliceControllerWidgetEventTranslatorPlayerTest1.cxx
    qDMMLSliceWidgetEventTranslatorPlayerTest1.cxx
    qDMMLThreeDViewControllerWidgetEventTranslatorPlayerTest1.cxx
    qDMMLThreeDWidgetEventTranslatorPlayerTest1.cxx
    qDMMLTransformSlidersEventTranslatorPlayerTest1.cxx
    qDMMLTreeViewEventTranslatorPlayerTest1.cxx
    qDMMLVolumeInfoWidgetEventTranslatorPlayerTest1.cxx
    qDMMLVolumeThresholdWidgetEventTranslatorPlayerTest1.cxx
    qDMMLWindowLevelWidgetEventTranslatorPlayerTest1.cxx
    )
endif()

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  )

set(Tests_MOC_SRCS qDMMLNodeObject.h)
set(Tests_UtilityFiles
  qDMMLNodeObject.cxx
  qDMMLNodeObject.h
  )

set(KIT_TEST_GENERATE_MOC_SRCS
  qDMMLCheckableNodeComboBoxTest.cxx
  qDMMLCollapsibleButtonTest.cxx
  qDMMLColorModelTest.cxx
  qDMMLDisplayNodeViewComboBoxTest.cxx
  qDMMLLabelComboBoxTest.cxx
  qDMMLLayoutManagerWithCustomFactoryTest.cxx
  qDMMLNodeAttributeTableViewTest.cxx
  qDMMLNodeAttributeTableWidgetTest.cxx
  qDMMLSceneModelTest.cxx
  qDMMLSliceControllerWidgetTest.cxx
  )
  set(_moc_options OPTIONS -DDMML_WIDGETS_HAVE_QT5)
  QT5_WRAP_CPP(Tests_MOC_CXX ${Tests_MOC_SRCS} ${_moc_options})
  QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})

#-----------------------------------------------------------------------------
set(INPUT ${DMMLCore_SOURCE_DIR}/Testing/TestData)

#-----------------------------------------------------------------------------
function(SCENE_TEST TESTNAME SCENEFILENAME)
  # Extract list of external files to download. Note that the ${_externalfiles} variable
  # is only specified to trigger download of data files used in the scene, the arguments
  # are NOT explcitly used in ${KIT}CxxTests test executable.
  string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
  list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
  list(LENGTH scenefilename_and_externalfiles _length)
  if(_length EQUAL 2)
    configure_file(
      ${DMMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
      ${DMMLCore_BINARY_DIR}/Testing/${SCENEFILENAME}
      COPYONLY
      )
    set(_scenefile ${DMMLCore_BINARY_DIR}/Testing/${SCENEFILENAME})
    list(GET scenefilename_and_externalfiles 1 _externalfiles)
  else()
    set(_scenefile ${DMMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME})
    set(_externalfiles "")
  endif()
  simple_test(${TESTNAME}_${SCENEFILENAME} DRIVER_TESTNAME ${TESTNAME} ${_scenefile} ${_externalfiles})
endfunction()

#-----------------------------------------------------------------------------
simple_test( qDMMLCheckableNodeComboBoxTest )
simple_test( qDMMLCheckableNodeComboBoxTest1 )
simple_test( qDMMLClipNodeWidgetTest1 )
simple_test( qDMMLCollapsibleButtonTest)
simple_test( qDMMLColorListViewTest1 )
simple_test( qDMMLColorModelTest )
simple_test( qDMMLColorModelTest1 )
simple_test( qDMMLColorPickerWidgetTest1 )
simple_test( qDMMLColorPickerWidgetTest2 )
simple_test( qDMMLColorPickerWidgetTest3 )
simple_test( qDMMLColorTableComboBoxTest1 )
simple_test( qDMMLColorTableViewTest1 )
simple_test( qDMMLDisplayNodeViewComboBoxTest )
simple_test( qDMMLDisplayNodeWidgetTest1 )
simple_test( qDMMLLayoutManagerTest1 )
simple_test( qDMMLLayoutManagerTest2 )
simple_test( qDMMLLayoutManagerTest3 )
simple_test( qDMMLLayoutManagerTest4 )
simple_test( qDMMLLayoutManagerVisibilityTest )
simple_test( qDMMLLayoutManagerWithCustomFactoryTest )
simple_test( qDMMLLinearTransformSliderTest1 )
simple_test( qDMMLListWidgetTest1 )
simple_test( qDMMLMatrixWidgetTest1 )
simple_test( qDMMLModelInfoWidgetTest1 )
simple_test( qDMMLModelTest1 )
simple_test( qDMMLModelTreeViewTest1 )
simple_test( qDMMLLabelComboBoxTest )
simple_test( qDMMLNavigationViewTest1 )
simple_test( qDMMLNodeAttributeTableViewTest )
simple_test( qDMMLNodeAttributeTableWidgetTest )
simple_test( qDMMLNodeComboBoxTest1 )
simple_test( qDMMLNodeComboBoxTest2 )
SCENE_TEST(  qDMMLNodeComboBoxTest3 diffusionEditor.dmml|DATA{${INPUT}/helix-DTI.nhdr,helix-DTI.raw.gz,helix-DWI.nhdr,helix-DWI.raw.gz} )
simple_test( qDMMLNodeComboBoxTest4 )
simple_test( qDMMLNodeComboBoxTest5 )
simple_test( qDMMLNodeComboBoxTest6 )
simple_test( qDMMLNodeComboBoxTest7 )
simple_test( qDMMLNodeComboBoxTest8 )
simple_test( qDMMLNodeComboBoxTest9 )
simple_test( qDMMLNodeComboBoxLazyUpdateTest1 )
simple_test( qDMMLNodeFactoryTest1 )
simple_test( qDMMLPlotViewTest1 )
simple_test( qDMMLScalarInvariantComboBoxTest1 )
simple_test( qDMMLSceneCategoryModelTest1 )
simple_test( qDMMLSceneColorTableModelTest1 )
simple_test( qDMMLSceneFactoryWidgetTest1 )
simple_test( qDMMLSceneModelTest )
simple_test( qDMMLSceneModelTest1 )
simple_test( qDMMLSceneTransformModelTest1 )
SCENE_TEST(  qDMMLSceneTransformModelTest2 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk} )
simple_test( qDMMLSceneDisplayableModelTest1 )
SCENE_TEST(  qDMMLSceneDisplayableModelTest2 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk} )
simple_test( qDMMLSliceControllerWidgetTest )
SCENE_TEST( qDMMLSliceWidgetTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
simple_test( qDMMLSliceWidgetTest2_fixed.nrrd DRIVER_TESTNAME qDMMLSliceWidgetTest2 DATA{${INPUT}/fixed.nrrd})
simple_test( qDMMLTableViewTest1 )
simple_test( qDMMLTransformSlidersTest1 )
simple_test( qDMMLThreeDViewTest1 )
simple_test( qDMMLThreeDWidgetTest1 )
SCENE_TEST(  qDMMLTreeViewTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk} )
SCENE_TEST(  qDMMLUtf8Test1 cube-utf8.dmml|DATA{${INPUT}/fixed.nrrd} )
simple_test( qDMMLUtilsTest1 )
simple_test( qDMMLVolumeInfoWidgetTest1 )
SCENE_TEST( qDMMLVolumeThresholdWidgetTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
SCENE_TEST( qDMMLVolumeThresholdWidgetTest2 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
simple_test( qDMMLWidgetsExportTest1 )
SCENE_TEST( qDMMLWindowLevelWidgetTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})

if(Cjyx_USE_QtTesting)
  function(QtTesting_TEST testname)
    if(${ARGC} GREATER 1)
      set(SCENEFILENAME ${ARGV1})

      # Extract list of external files to download. Note that the ${_externalfiles} variable
      # is only specified to trigger download of data files used in the scene, the arguments
      # are NOT explcitly used in ${KIT}CxxTests test executable.
      string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
      list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
      list(LENGTH scenefilename_and_externalfiles _length)
      if(_length EQUAL 2)
        configure_file(
          ${DMMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
          ${DMMLCore_BINARY_DIR}/Testing/${SCENEFILENAME}
          COPYONLY
          )
        set(_scenefile ${DMMLCore_BINARY_DIR}/Testing/${SCENEFILENAME})
        list(GET scenefilename_and_externalfiles 1 _externalfiles)
      else()
        set(_scenefile ${DMMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME})
        set(_externalfiles "")
      endif()

    else()
      set(_scenefile "")
    endif()
    simple_test(${testname} ${Cjyx_SOURCE_DIR} ${_scenefile})
    set_tests_properties(${testname} PROPERTIES RUN_SERIAL ON)
  endfunction()
  QtTesting_TEST(qDMMLCheckableNodeComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLClipNodeWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLCollapsibleButtonEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLColorListViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLColorPickerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLColorTableComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLColorTableViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLDisplayNodeWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLLabelComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLLinearTransformSliderEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLListWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLMatrixWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLNavigationViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLNodeComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLRangeWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLROIWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLScalarInvariantComboBoxEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLSliceControllerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLSliceWidgetEventTranslatorPlayerTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
  QtTesting_TEST(qDMMLThreeDViewControllerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLThreeDWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLTransformSlidersEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLTreeViewEventTranslatorPlayerTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
  QtTesting_TEST(qDMMLVolumeInfoWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qDMMLVolumeThresholdWidgetEventTranslatorPlayerTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
  QtTesting_TEST(qDMMLWindowLevelWidgetEventTranslatorPlayerTest1 vol_and_cube.dmml|DATA{${INPUT}/fixed.nrrd,cube.vtk})
endif()
