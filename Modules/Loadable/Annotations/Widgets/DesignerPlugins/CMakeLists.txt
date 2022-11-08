
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_CJYX_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qCjyxAnnotationModuleWidgetsAbstractPlugin.cxx
  qCjyxAnnotationModuleWidgetsAbstractPlugin.h
  qCjyxAnnotationModuleWidgetsPlugin.cxx
  qCjyxAnnotationModuleWidgetsPlugin.h

  qDMMLAnnotationFiducialProjectionPropertyWidgetPlugin.cxx
  qDMMLAnnotationFiducialProjectionPropertyWidgetPlugin.h
  qDMMLAnnotationROIWidgetPlugin.cxx
  qDMMLAnnotationROIWidgetPlugin.h
  qDMMLAnnotationRulerProjectionPropertyWidgetPlugin.cxx
  qDMMLAnnotationRulerProjectionPropertyWidgetPlugin.h
  qDMMLAnnotationTreeViewPlugin.cxx
  qDMMLAnnotationTreeViewPlugin.h
  )

set(${KIT}_MOC_SRCS
  qCjyxAnnotationModuleWidgetsPlugin.h

  qDMMLAnnotationFiducialProjectionPropertyWidgetPlugin.h
  qDMMLAnnotationROIWidgetPlugin.h
  qDMMLAnnotationRulerProjectionPropertyWidgetPlugin.h
  qDMMLAnnotationTreeViewPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qCjyxAnnotationsModuleWidgets
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
