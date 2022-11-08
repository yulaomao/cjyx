set(KIT qCjyx${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkDMMLSequenceBrowserNodeTest1.cxx
  vtkDMMLSequenceNodeTest1.cxx
  vtkDMMLSequenceStorageNodeTest1.cxx
  )

#-----------------------------------------------------------------------------
cjyxMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
simple_test(vtkDMMLSequenceBrowserNodeTest1)
simple_test(vtkDMMLSequenceNodeTest1)
simple_test(vtkDMMLSequenceStorageNodeTest1 ${TEMP})
