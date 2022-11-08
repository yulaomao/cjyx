set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(INPUT ${DMMLCore_SOURCE_DIR}/Testing)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qDMMLModelDisplayNodeWidgetTest1.cxx
  qDMMLModelDisplayNodeWidgetTest2.cxx
  )

#-----------------------------------------------------------------------------
cjyxMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( qDMMLModelDisplayNodeWidgetTest1 )
simple_test( qDMMLModelDisplayNodeWidgetTest2 ${INPUT}/cube.dmml )
