project(qCjyx${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_CJYX_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkCjyx${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleLogic_BINARY_DIR}
  )

set(${KIT}_SRCS
  qDMMLPlotSeriesPropertiesWidget_p.h
  qDMMLPlotSeriesPropertiesWidget.h
  qDMMLPlotSeriesPropertiesWidget.cxx
  qDMMLPlotChartPropertiesWidget_p.h
  qDMMLPlotChartPropertiesWidget.h
  qDMMLPlotChartPropertiesWidget.cxx
  )

set(${KIT}_MOC_SRCS
  qDMMLPlotSeriesPropertiesWidget.h
  qDMMLPlotChartPropertiesWidget.h
  qDMMLPlotChartPropertiesWidget_p.h
 qDMMLPlotSeriesPropertiesWidget_p.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qDMMLPlotSeriesPropertiesWidget.ui
  Resources/UI/qDMMLPlotChartPropertiesWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
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
if(Cjyx_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
