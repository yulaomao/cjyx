set(KIT qCjyx${MODULE_NAME}Module)

#-----------------------------------------------------------------------------

#
# ITK
#
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
set(DMML_CORE_INPUT "${DMMLCore_SOURCE_DIR}/Testing/TestData/")
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)
set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qDMMLVolumePropertyNodeWidgetTest1.cxx
  qCjyxPresetComboBoxTest.cxx
  qCjyx${MODULE_NAME}ModuleWidgetTest1.cxx
  qCjyx${MODULE_NAME}ModuleWidgetTest2.cxx
  vtkDMMLShaderPropertyStorageNodeTest1.cxx
  vtkDMMLVolumePropertyNodeTest1.cxx
  vtkDMMLVolumePropertyStorageNodeTest1.cxx
  vtkDMMLVolumeRenderingDisplayableManagerTest1.cxx
  vtkDMMLVolumeRenderingMultiVolumeTest.cxx
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_GENERATE_MOC_SRCS
  qCjyxPresetComboBoxTest.cxx
  )
QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
cjyxMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkCjyxVolumesModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qDMMLVolumePropertyNodeWidgetTest1)
simple_test(qCjyxPresetComboBoxTest)
simple_test(qCjyx${MODULE_NAME}ModuleWidgetTest1)
simple_test(qCjyx${MODULE_NAME}ModuleWidgetTest2 DATA{${DMML_CORE_INPUT}/fixed.nrrd})
simple_test(vtkDMMLShaderPropertyStorageNodeTest1 ${TEMP})
simple_test(vtkDMMLVolumePropertyNodeTest1 ${INPUT}/volRender.dmml)
simple_test(vtkDMMLVolumePropertyStorageNodeTest1)
simple_test(vtkDMMLVolumeRenderingDisplayableManagerTest1 ${CMAKE_BINARY_DIR}/${Cjyx_QTLOADABLEMODULES_SHARE_DIR}/VolumeRendering)
simple_test(vtkDMMLVolumeRenderingMultiVolumeTest)
