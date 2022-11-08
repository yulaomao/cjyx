project(qCjyx${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_CJYX_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkCjyx${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleLogic_BINARY_DIR}
  )

set(${KIT}_SRCS
  qDMMLSequenceBrowserPlayWidget.cxx
  qDMMLSequenceBrowserPlayWidget.h
  qDMMLSequenceBrowserSeekWidget.cxx
  qDMMLSequenceBrowserSeekWidget.h
  qDMMLSequenceBrowserToolBar.cxx
  qDMMLSequenceBrowserToolBar.h
  )

set(${KIT}_MOC_SRCS
  qDMMLSequenceBrowserPlayWidget.h
  qDMMLSequenceBrowserSeekWidget.h
  qDMMLSequenceBrowserToolBar.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qDMMLSequenceBrowserPlayWidget.ui
  Resources/UI/qDMMLSequenceBrowserSeekWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/qCjyx${MODULE_NAME}ModuleWidgets.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  vtkCjyx${MODULE_NAME}ModuleDMML
  vtkCjyx${MODULE_NAME}ModuleLogic
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
add_subdirectory(DesignerPlugins)

# Testing
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
