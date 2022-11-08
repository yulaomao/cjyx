set(KIT qCjyx${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(TEMP ${Cjyx_BINARY_DIR}/Testing/Temporary)
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkDMMLMarkupsDisplayNodeTest1.cxx
  vtkDMMLMarkupsFiducialNodeTest1.cxx
  vtkDMMLMarkupsNodeTest1.cxx
  vtkDMMLMarkupsNodeTest2.cxx
  vtkDMMLMarkupsNodeTest3.cxx
  vtkDMMLMarkupsNodeTest4.cxx
  vtkDMMLMarkupsNodeTest5.cxx
  vtkDMMLMarkupsNodeTest6.cxx
  vtkDMMLMarkupsFiducialStorageNodeTest2.cxx
  vtkDMMLMarkupsFiducialStorageNodeTest3.cxx
  vtkDMMLMarkupsStorageNodeTest1.cxx
  vtkDMMLMarkupsStorageNodeTest2.cxx
  vtkCjyxMarkupsLogicTest1.cxx
  vtkCjyxMarkupsLogicTest2.cxx
  vtkCjyxMarkupsLogicTest3.cxx
  vtkCjyxMarkupsLogicTest4.cxx
  vtkMarkupsAnnotationSceneTest.cxx
  vtkDMMLMarkupsNodeEventsTest.cxx
  )

#-----------------------------------------------------------------------------
cjyxMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES
    vtkCjyxAnnotationsModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

SIMPLE_TEST( vtkDMMLMarkupsDisplayNodeTest1 )
SIMPLE_TEST( vtkDMMLMarkupsFiducialNodeTest1 )
SIMPLE_TEST( vtkDMMLMarkupsNodeTest1 )
SIMPLE_TEST( vtkDMMLMarkupsNodeTest2 )
SIMPLE_TEST( vtkDMMLMarkupsNodeTest3 )
SIMPLE_TEST( vtkDMMLMarkupsNodeTest4 )
SIMPLE_TEST( vtkDMMLMarkupsNodeTest5 )
SIMPLE_TEST( vtkDMMLMarkupsNodeTest6 )
SIMPLE_TEST( vtkDMMLMarkupsNodeEventsTest )

# test legacy Cjyx3 fcsv file
SIMPLE_TEST( vtkDMMLMarkupsFiducialStorageNodeTest2 ${INPUT}/cjyx3.fcsv )

# test Cjyx4 annotation acsv file
SIMPLE_TEST( vtkDMMLMarkupsFiducialStorageNodeTest3 ${INPUT}/cjyx4.acsv )

SIMPLE_TEST( vtkDMMLMarkupsStorageNodeTest1 )
SIMPLE_TEST( vtkDMMLMarkupsStorageNodeTest2 ${TEMP} )

# logic tests
SIMPLE_TEST( vtkCjyxMarkupsLogicTest1 )
SIMPLE_TEST( vtkCjyxMarkupsLogicTest2 ${TEMP} )
SIMPLE_TEST( vtkCjyxMarkupsLogicTest3 )
SIMPLE_TEST( vtkCjyxMarkupsLogicTest4 )

# test Cjyx4 annotation fiducials in a dmml file
# TODO: remove this after annotation fiducials have been removed
SIMPLE_TEST( vtkMarkupsAnnotationSceneTest ${INPUT}/AnnotationTest/AnnotationFiducialsTest.dmml )
