project(qCjyx${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_CJYX_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkCjyx${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleLogic_BINARY_DIR}
  )

set(${KIT}_SRCS
  #qDMMLAnnotationDisplayNodePropertyWidget.cxx
  #qDMMLAnnotationDisplayNodePropertyWidget.h
  #qDMMLAnnotationDisplayNodePointPropertyWidget.cxx
  #qDMMLAnnotationDisplayNodePointPropertyWidget.h
  #qDMMLAnnotationDisplayNodePointPropertyWidget_p.h
  qDMMLAnnotationFiducialProjectionPropertyWidget.cxx
  qDMMLAnnotationFiducialProjectionPropertyWidget.h
  qDMMLAnnotationROIWidget.cxx
  qDMMLAnnotationROIWidget.h
  qDMMLAnnotationRulerProjectionPropertyWidget.cxx
  qDMMLAnnotationRulerProjectionPropertyWidget.h
  qDMMLAnnotationTreeView.cxx
  qDMMLAnnotationTreeView.h
  qDMMLSceneAnnotationModel.cxx
  qDMMLSceneAnnotationModel.h
  )

set(${KIT}_MOC_SRCS
  #qDMMLAnnotationDisplayNodePropertyWidget.h
  #qDMMLAnnotationDisplayNodePointPropertyWidget.h
  #qDMMLAnnotationDisplayNodePointPropertyWidget_p.h
  qDMMLAnnotationFiducialProjectionPropertyWidget.h
  qDMMLAnnotationROIWidget.h
  qDMMLAnnotationRulerProjectionPropertyWidget.h
  qDMMLAnnotationTreeView.h
  qDMMLSceneAnnotationModel.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qDMMLAnnotationFiducialProjectionPropertyWidget.ui
  Resources/UI/qDMMLAnnotationROIWidget.ui
  Resources/UI/qDMMLAnnotationRulerProjectionPropertyWidget.ui
  #Resources/UI/qDMMLAnnotationDisplayNodePointPropertyWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkCjyxAnnotationsModuleDMML
  vtkCjyxAnnotationsModuleLogic
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
