set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(INPUT ${DMMLCore_SOURCE_DIR}/Testing)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qDMMLSequenceBrowserWidgetsTest1.cxx
  )

#-----------------------------------------------------------------------------
cjyxMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( qDMMLSequenceBrowserWidgetsTest1 )
