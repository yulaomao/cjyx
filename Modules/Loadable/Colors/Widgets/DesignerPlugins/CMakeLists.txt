
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_CJYX_QTMODULES_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qDMMLColorLegendDisplayNodeWidgetPlugin.cxx
  qDMMLColorLegendDisplayNodeWidgetPlugin.h
#  qDMMLTextPropertyWidgetPlugin.cxx
#  qDMMLTextPropertyWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qDMMLColorLegendDisplayNodeWidgetPlugin.h
#  qDMMLTextPropertyWidgetPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qCjyx${MODULE_NAME}ModuleWidgets
  )

#-----------------------------------------------------------------------------
ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Module-${MODULE_NAME}"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
