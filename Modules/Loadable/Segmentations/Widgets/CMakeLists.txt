project(qCjyx${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_CJYX_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkCjyxSegmentationsModuleDMML_INCLUDE_DIRS}
  ${vtkCjyxSegmentationsModuleLogic_INCLUDE_DIRS}
  ${qCjyxSegmentationsEditorEffects_INCLUDE_DIRS}
  ${qCjyxTerminologiesModuleWidgets_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qDMMLDoubleSpinBoxDelegate.h
  qDMMLDoubleSpinBoxDelegate.cxx
  qDMMLSegmentsModel.cxx
  qDMMLSegmentsModel.h
  qDMMLSegmentsModel_p.h
  qDMMLSegmentsTableView.cxx
  qDMMLSegmentsTableView.h
  qDMMLSegmentationConversionParametersWidget.cxx
  qDMMLSegmentationConversionParametersWidget.h
  qDMMLSegmentationDisplayNodeWidget.cxx
  qDMMLSegmentationDisplayNodeWidget.h
  qDMMLSegmentationFileExportWidget.cxx
  qDMMLSegmentationFileExportWidget.h
  qDMMLSegmentationGeometryWidget.cxx
  qDMMLSegmentationGeometryWidget.h
  qDMMLSegmentationGeometryDialog.cxx
  qDMMLSegmentationGeometryDialog.h
  qDMMLSegmentationRepresentationsListView.cxx
  qDMMLSegmentationRepresentationsListView.h
  qDMMLSegmentationShow3DButton.h
  qDMMLSegmentationShow3DButton.cxx
  qDMMLSegmentEditorWidget.cxx
  qDMMLSegmentEditorWidget.h
  qDMMLSortFilterSegmentsProxyModel.cxx
  qDMMLSortFilterSegmentsProxyModel.h
  )

set(${KIT}_MOC_SRCS
  qDMMLSegmentsTableView.h
  qDMMLSegmentsModel.h
  qDMMLSegmentationConversionParametersWidget.h
  qDMMLSegmentationDisplayNodeWidget.h
  qDMMLSegmentationFileExportWidget.h
  qDMMLSegmentationGeometryWidget.h
  qDMMLSegmentationGeometryDialog.h
  qDMMLSegmentationRepresentationsListView.h
  qDMMLSegmentationShow3DButton.h
  qDMMLSegmentEditorWidget.h
  qDMMLSortFilterSegmentsProxyModel.h
  qDMMLDoubleSpinBoxDelegate.h
)

set(${KIT}_UI_SRCS
  Resources/UI/qDMMLSegmentationRepresentationsListView.ui
  Resources/UI/qDMMLSegmentationConversionParametersWidget.ui
  Resources/UI/qDMMLSegmentEditorWidget.ui
  Resources/UI/qDMMLSegmentationDisplayNodeWidget.ui
  Resources/UI/qDMMLSegmentationFileExportWidget.ui
  Resources/UI/qDMMLSegmentationGeometryWidget.ui
  Resources/UI/qDMMLSegmentsTableView.ui
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  qCjyxSegmentationsEditorEffects
  qCjyxTerminologiesModuleWidgets
  vtkCjyxSegmentationsModuleDMML
  vtkCjyxSegmentationsModuleLogic
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
  #add_subdirectory(Testing)
endif()
