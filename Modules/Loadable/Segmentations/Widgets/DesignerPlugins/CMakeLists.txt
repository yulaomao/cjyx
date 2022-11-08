set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_CJYX_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qCjyxSegmentationsModuleWidgetsAbstractPlugin.cxx
  qCjyxSegmentationsModuleWidgetsAbstractPlugin.h
  qCjyxSegmentationsModuleWidgetsPlugin.cxx
  qCjyxSegmentationsModuleWidgetsPlugin.h
  qDMMLSegmentsTableViewPlugin.cxx
  qDMMLSegmentsTableViewPlugin.h
  qDMMLSegmentationRepresentationsListViewPlugin.cxx
  qDMMLSegmentationRepresentationsListViewPlugin.h
  qDMMLSegmentationConversionParametersWidgetPlugin.cxx
  qDMMLSegmentationConversionParametersWidgetPlugin.h
  qDMMLSegmentationShow3DButtonPlugin.cxx
  qDMMLSegmentationShow3DButtonPlugin.h
  qDMMLSegmentSelectorWidgetPlugin.cxx
  qDMMLSegmentSelectorWidgetPlugin.h
  qDMMLSegmentEditorWidgetPlugin.cxx
  qDMMLSegmentEditorWidgetPlugin.h
  qDMMLSegmentationDisplayNodeWidgetPlugin.cxx
  qDMMLSegmentationDisplayNodeWidgetPlugin.h
  qDMMLSegmentationFileExportWidgetPlugin.cxx
  qDMMLSegmentationFileExportWidgetPlugin.h
  )

set(${KIT}_MOC_SRCS
  qCjyxSegmentationsModuleWidgetsPlugin.h
  qDMMLSegmentsTableViewPlugin.h
  qDMMLSegmentationRepresentationsListViewPlugin.h
  qDMMLSegmentationConversionParametersWidgetPlugin.h
  qDMMLSegmentSelectorWidgetPlugin.h
  qDMMLSegmentEditorWidgetPlugin.h
  qDMMLSegmentationDisplayNodeWidgetPlugin.h
  qDMMLSegmentationFileExportWidgetPlugin.h
  qDMMLSegmentationShow3DButtonPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qCjyxSegmentationsModuleWidgets
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
