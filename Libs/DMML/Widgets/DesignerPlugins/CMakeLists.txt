set(KIT ${PROJECT_NAME})
project(${KIT}Plugins)

#
# See CTK/CMake/ctkMacroBuildQtPlugin.cmake for details
#

# The header '${KIT}PluginsExport.h' will be automatically configured.
set(${KIT}_EXPORT "QDMML_WIDGETS_PLUGINS_EXPORT")

# Source files
set(${KIT}_SRCS
  qDMMLWidgetsAbstractPlugin.cxx
  qDMMLWidgetsAbstractPlugin.h
  qDMMLWidgetsPlugin.cxx
  qDMMLWidgetsPlugin.h

  qDMMLCaptureToolBarPlugin.h
  qDMMLCaptureToolBarPlugin.cxx
  qDMMLCheckableNodeComboBoxPlugin.cxx
  qDMMLCheckableNodeComboBoxPlugin.h
  qDMMLClipNodeWidgetPlugin.cxx
  qDMMLClipNodeWidgetPlugin.h
  qDMMLCollapsibleButtonPlugin.cxx
  qDMMLCollapsibleButtonPlugin.h
  qDMMLColorListViewPlugin.cxx
  qDMMLColorListViewPlugin.h
  qDMMLColorTableComboBoxPlugin.cxx
  qDMMLColorTableComboBoxPlugin.h
  qDMMLColorTableViewPlugin.cxx
  qDMMLColorTableViewPlugin.h
  qDMMLCoordinatesWidgetPlugin.cxx
  qDMMLCoordinatesWidgetPlugin.h
  qDMMLDisplayNodeViewComboBoxPlugin.cxx
  qDMMLDisplayNodeViewComboBoxPlugin.h
  qDMMLDisplayNodeWidgetPlugin.cxx
  qDMMLDisplayNodeWidgetPlugin.h
  qDMMLEventBrokerWidgetPlugin.cxx
  qDMMLEventBrokerWidgetPlugin.h
  qDMMLLabelComboBoxPlugin.cxx
  qDMMLLabelComboBoxPlugin.h
  qDMMLLayoutWidgetPlugin.cxx
  qDMMLLayoutWidgetPlugin.h
  qDMMLLinearTransformSliderPlugin.cxx
  qDMMLLinearTransformSliderPlugin.h
  qDMMLListWidgetPlugin.cxx
  qDMMLListWidgetPlugin.h
  qDMMLMatrixWidgetPlugin.cxx
  qDMMLMatrixWidgetPlugin.h
  qDMMLModelInfoWidgetPlugin.cxx
  qDMMLModelInfoWidgetPlugin.h
  qDMMLNavigationViewPlugin.cxx
  qDMMLNavigationViewPlugin.h
  qDMMLNodeAttributeTableViewPlugin.cxx
  qDMMLNodeAttributeTableViewPlugin.h
  qDMMLNodeAttributeTableWidgetPlugin.cxx
  qDMMLNodeAttributeTableWidgetPlugin.h
  qDMMLNodeComboBoxPlugin.cxx
  qDMMLNodeComboBoxPlugin.h
  qDMMLPlotViewControllerWidgetPlugin.cxx
  qDMMLPlotViewControllerWidgetPlugin.h
  qDMMLPlotWidgetPlugin.cxx
  qDMMLPlotWidgetPlugin.h
  qDMMLRangeWidgetPlugin.cxx
  qDMMLRangeWidgetPlugin.h
  qDMMLROIWidgetPlugin.cxx
  qDMMLROIWidgetPlugin.h
  qDMMLScalarInvariantComboBoxPlugin.cxx
  qDMMLScalarInvariantComboBoxPlugin.h
  qDMMLScalarsDisplayWidgetPlugin.cxx
  qDMMLScalarsDisplayWidgetPlugin.h
  qDMMLSceneFactoryWidgetPlugin.cxx
  qDMMLSceneFactoryWidgetPlugin.h
  qDMMLSliceControllerWidgetPlugin.cxx
  qDMMLSliceControllerWidgetPlugin.h
  qDMMLSliceInformationWidgetPlugin.cxx
  qDMMLSliceInformationWidgetPlugin.h
  qDMMLSliceWidgetPlugin.cxx
  qDMMLSliceWidgetPlugin.h
  qDMMLSliderWidgetPlugin.cxx
  qDMMLSliderWidgetPlugin.h
  qDMMLSpinBoxPlugin.cxx
  qDMMLSpinBoxPlugin.h
  qDMMLTableViewPlugin.cxx
  qDMMLTableViewPlugin.h
  qDMMLThreeDViewInformationWidgetPlugin.cxx
  qDMMLThreeDViewInformationWidgetPlugin.h
  qDMMLThreeDViewPlugin.cxx
  qDMMLThreeDViewPlugin.h
  qDMMLTransformSlidersPlugin.cxx
  qDMMLTransformSlidersPlugin.h
  qDMMLTreeViewPlugin.cxx
  qDMMLTreeViewPlugin.h
  qDMMLVolumeInfoWidgetPlugin.cxx
  qDMMLVolumeInfoWidgetPlugin.h
  qDMMLVolumeThresholdWidgetPlugin.cxx
  qDMMLVolumeThresholdWidgetPlugin.h
  qDMMLWidgetPlugin.cxx
  qDMMLWidgetPlugin.h
  qDMMLWindowLevelWidgetPlugin.cxx
  qDMMLWindowLevelWidgetPlugin.h
  )

if(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  list(APPEND ${KIT}_SRCS
    qDMMLExpandingWebViewPlugin.cxx
    qDMMLExpandingWebViewPlugin.h
    )
endif()

# Headers that should run through moc
set(${KIT}_MOC_SRCS
  qDMMLWidgetsPlugin.h

  qDMMLCaptureToolBarPlugin.h
  qDMMLCheckableNodeComboBoxPlugin.h
  qDMMLClipNodeWidgetPlugin.h
  qDMMLCollapsibleButtonPlugin.h
  qDMMLColorListViewPlugin.h
  qDMMLColorTableComboBoxPlugin.h
  qDMMLColorTableViewPlugin.h
  qDMMLCoordinatesWidgetPlugin.h
  qDMMLDisplayNodeViewComboBoxPlugin.h
  qDMMLDisplayNodeWidgetPlugin.h
  qDMMLEventBrokerWidgetPlugin.h
  qDMMLLabelComboBoxPlugin.h
  qDMMLLayoutWidgetPlugin.h
  qDMMLLinearTransformSliderPlugin.h
  qDMMLListWidgetPlugin.h
  qDMMLMatrixWidgetPlugin.h
  qDMMLModelInfoWidgetPlugin.h
  qDMMLNavigationViewPlugin.h
  qDMMLNodeAttributeTableViewPlugin.h
  qDMMLNodeAttributeTableWidgetPlugin.h
  qDMMLNodeComboBoxPlugin.h
  qDMMLPlotViewControllerWidgetPlugin.h
  qDMMLPlotWidgetPlugin.h
  qDMMLRangeWidgetPlugin.h
  qDMMLROIWidgetPlugin.h
  qDMMLScalarInvariantComboBoxPlugin.h
  qDMMLScalarsDisplayWidgetPlugin.h
  qDMMLSceneFactoryWidgetPlugin.h
  qDMMLSliceControllerWidgetPlugin.h
  qDMMLSliceInformationWidgetPlugin.h
  qDMMLSliceWidgetPlugin.h
  qDMMLSliderWidgetPlugin.h
  qDMMLSpinBoxPlugin.h
  qDMMLTableViewPlugin.h
  qDMMLThreeDViewInformationWidgetPlugin.h
  qDMMLThreeDViewPlugin.h
  qDMMLTransformSlidersPlugin.h
  qDMMLTreeViewPlugin.h
  qDMMLVolumeInfoWidgetPlugin.h
  qDMMLVolumeThresholdWidgetPlugin.h
  qDMMLWidgetPlugin.h
  qDMMLWindowLevelWidgetPlugin.h
  )

if(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  list(APPEND ${KIT}_MOC_SRCS
    qDMMLExpandingWebViewPlugin.h
    )
endif()

set(${KIT}_TARGET_LIBRARIES
  qDMMLWidgets
  )

ctkMacroBuildQtDesignerPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "${${KIT}_FOLDER}"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
