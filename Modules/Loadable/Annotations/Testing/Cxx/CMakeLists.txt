set(KIT qCjyx${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(TEMP "${Cjyx_BINARY_DIR}/Testing/Temporary")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/Fiducials\ List.acsv
  ${TEMP}/Fiducials\ List.acsv
  COPYONLY
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  #qDMMLSceneAnnotationModelAndAnnotationTreeWidgetTest1
  qDMMLAnnotationROIWidgetTest1.cxx
  vtkDMMLAnnotationNodeTest1.cxx
  vtkDMMLAnnotationNodesUndoTest1.cxx
  vtkDMMLAnnotationStorageNodeTest1.cxx
  vtkDMMLAnnotationDisplayNodeTest1.cxx
  vtkDMMLAnnotationDisplayNodeTest2.cxx
  vtkDMMLAnnotationControlPointsNodeTest1.cxx
  vtkDMMLAnnotationPointDisplayNodeTest1.cxx
  vtkDMMLAnnotationControlPointsStorageNodeTest1.cxx
  #vtkDMMLAnnotationLinesNodeTest1.cxx
  vtkDMMLAnnotationLineDisplayNodeTest1.cxx
  #vtkDMMLAnnotationLinesStorageNodeTest1.cxx
  vtkDMMLAnnotationRulerNodeTest1.cxx
  vtkDMMLAnnotationRulerStorageNodeTest1.cxx
  vtkDMMLAnnotationFiducialNodeTest1.cxx
  vtkDMMLAnnotationFiducialsStorageNodeTest1.cxx
  vtkDMMLAnnotationROINodeTest1.cxx
  vtkDMMLAnnotationDisplayableManagerTest1.cxx
  vtkDMMLAnnotationDisplayableManagerTest2.cxx
  #vtkDMMLAnnotationRulerDisplayableManagerTest1.cxx
  vtkCjyxAnnotationModuleLogicTest1.cxx
  vtkCjyxAnnotationModuleLogicImportSceneTest.cxx
  #qCjyxAnnotationModuleAnnotationPropertyDialogTest1.cxx
  )

#-----------------------------------------------------------------------------
cjyxMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES qCjyx${MODULE_NAME}ModuleWidgets
  TESTS_TO_RUN_VAR KIT_TESTS_TO_RUN
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
# Add all the tests
foreach(test ${KIT_TESTS_TO_RUN})
  get_filename_component(testname ${test} NAME_WE)
  simple_test(${testname}
      -D ${INPUT}
      -T ${TEMP}
      #-V Baseline/${testname}.png
    )
endforeach()

