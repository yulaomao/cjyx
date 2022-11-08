project(qDMMLWidgets)


#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)
#-----------------------------------------------------------------------------

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

if(NOT DEFINED DMML_BUILD_QT_DESIGNER_PLUGINS)
  option(DMML_BUILD_QT_DESIGNER_PLUGINS "Build Qt designer plugins." ON)
endif()

include(CheckIncludeFiles)
check_include_files("stdint.h" HAVE_STDINT_H)

# CMake module
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${DMMLWidgets_SOURCE_DIR}/CMake")

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------


#
# CTK
#
set(CTK_LIBRARIES CTKVisualizationVTKWidgets)
if(Cjyx_USE_PYTHONQT)
  list(APPEND CTK_LIBRARIES
    CTKScriptingPythonWidgets
    )
endif()
if(Cjyx_USE_QtTesting)
  list(APPEND CTK_LIBRARIES
    CTKQtTesting
    )
endif()

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(DMML_WIDGETS_HAVE_QT5 1)
if(NOT DEFINED DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  set(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT 1)
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/qDMMLWidgetsConfigure.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/qDMMLWidgetsConfigure.h
  )

#-----------------------------------------------------------------------------
# Sources
#
set(DMMLWidgets_SRCS
  qDMMLCaptureToolBar.cxx
  qDMMLCaptureToolBar.h
  qDMMLCheckableNodeComboBox.cxx
  qDMMLCheckableNodeComboBox.h
  qDMMLClipNodeWidget.cxx
  qDMMLClipNodeWidget.h
  qDMMLCollapsibleButton.cxx
  qDMMLCollapsibleButton.h
  qDMMLColors.cxx
  qDMMLColors.h
  qDMMLColorListView.cxx
  qDMMLColorListView.h
  qDMMLColorModel.cxx
  qDMMLColorModel.h
  qDMMLColorPickerWidget.cxx
  qDMMLColorPickerWidget.h
  qDMMLColorTableComboBox.cxx
  qDMMLColorTableComboBox.h
  qDMMLColorTableView.cxx
  qDMMLColorTableView.h
  qDMMLCoordinatesWidget.cxx
  qDMMLCoordinatesWidget.h
  qDMMLDisplayNodeWidget.cxx
  qDMMLDisplayNodeWidget.h
  qDMMLDisplayNodeViewComboBox.cxx
  qDMMLDisplayNodeViewComboBox.h
  qDMMLLabelComboBox.h
  qDMMLLabelComboBox.cxx
  qDMMLEventBrokerConnection.cxx
  qDMMLEventBrokerConnection.h
  qDMMLEventBrokerWidget.cxx
  qDMMLEventBrokerWidget.h
  qDMMLEventLogger.cxx
  qDMMLEventLogger.h
  qDMMLEventLoggerWidget.cxx
  qDMMLEventLoggerWidget.h
  qDMMLItemDelegate.cxx
  qDMMLItemDelegate.h
  qDMMLLayoutManager.cxx
  qDMMLLayoutManager.h
  qDMMLLayoutManager_p.h
  qDMMLLayoutViewFactory.cxx
  qDMMLLayoutViewFactory.h
  qDMMLLayoutWidget.cxx
  qDMMLLayoutWidget.h
  qDMMLLinearTransformSlider.cxx
  qDMMLLinearTransformSlider.h
  qDMMLListWidget.cxx
  qDMMLListWidget.h
  qDMMLMatrixWidget.cxx
  qDMMLMatrixWidget.h
  qDMMLModelInfoWidget.cxx
  qDMMLModelInfoWidget.h
  qDMMLNavigationView.cxx
  qDMMLNavigationView.h
  qDMMLNodeAttributeTableView.cxx
  qDMMLNodeAttributeTableView.h
  qDMMLNodeAttributeTableWidget.cxx
  qDMMLNodeAttributeTableWidget.h
  qDMMLNodeComboBox.cxx
  qDMMLNodeComboBox.h
  qDMMLNodeComboBoxDelegate.cxx
  qDMMLNodeComboBoxDelegate.h
  qDMMLNodeComboBoxMenuDelegate.cxx
  qDMMLNodeComboBoxMenuDelegate.h
  qDMMLNodeComboBox_p.h
  qDMMLNodeFactory.cxx
  qDMMLNodeFactory.h
  qDMMLPlotWidget.cxx
  qDMMLPlotWidget.h
  qDMMLPlotView.cxx
  qDMMLPlotView_p.h
  qDMMLPlotView.h
  qDMMLPlotViewControllerWidget.cxx
  qDMMLPlotViewControllerWidget.h
  qDMMLPlotViewControllerWidget_p.h
  qDMMLRangeWidget.cxx
  qDMMLRangeWidget.h
  qDMMLROIWidget.cxx
  qDMMLROIWidget.h
  qDMMLScalarInvariantComboBox.cxx
  qDMMLScalarInvariantComboBox.h
  qDMMLScalarsDisplayWidget.cxx
  qDMMLScalarsDisplayWidget.h
  qDMMLSceneCategoryModel.cxx
  qDMMLSceneCategoryModel.h
  qDMMLSceneColorTableModel.cxx
  qDMMLSceneColorTableModel.h
  qDMMLSceneFactoryWidget.cxx
  qDMMLSceneFactoryWidget.h
  qDMMLSceneModel.cxx
  qDMMLSceneModel.h
  qDMMLSceneViewMenu.cxx
  qDMMLSceneViewMenu.h
  qDMMLSceneViewMenu_p.h
  qDMMLSceneTransformModel.cxx
  qDMMLSceneTransformModel.h
  qDMMLSceneHierarchyModel.cxx
  qDMMLSceneHierarchyModel.h
  qDMMLSceneDisplayableModel.cxx
  qDMMLSceneDisplayableModel.h
  qDMMLScreenShotDialog.cxx
  qDMMLScreenShotDialog.h
  qDMMLSegmentSelectorWidget.cxx
  qDMMLSegmentSelectorWidget.h
  qDMMLSliceControllerWidget.cxx
  qDMMLSliceControllerWidget.h
  qDMMLSliceControllerWidget_p.h
  qDMMLSliceInformationWidget.cxx
  qDMMLSliceInformationWidget.h
  qDMMLSliceInformationWidget_p.h
  qDMMLSliceView.cxx
  qDMMLSliceView.h
  qDMMLSliceView_p.h
  qDMMLSliceWidget.cxx
  qDMMLSliceWidget.h
  qDMMLSliceWidget_p.h
  qDMMLSliderWidget.cxx
  qDMMLSliderWidget.h
  qDMMLSortFilterHierarchyProxyModel.cxx
  qDMMLSortFilterHierarchyProxyModel.h
  qDMMLSortFilterProxyModel.cxx
  qDMMLSortFilterProxyModel.h
  qDMMLSpinBox.cxx
  qDMMLSpinBox.h
  qDMMLTableModel.cxx
  qDMMLTableModel.h
  qDMMLTableView.cxx
  qDMMLTableView.h
  qDMMLTableView_p.h
  qDMMLTableViewControllerWidget.cxx
  qDMMLTableViewControllerWidget.h
  qDMMLTableViewControllerWidget_p.h
  qDMMLTableWidget.cxx
  qDMMLTableWidget.h
  qDMMLThreeDView.cxx
  qDMMLThreeDView.h
  qDMMLThreeDView_p.h
  qDMMLThreeDViewControllerWidget.cxx
  qDMMLThreeDViewControllerWidget.h
  qDMMLThreeDViewControllerWidget_p.h
  qDMMLThreeDViewInformationWidget.cxx
  qDMMLThreeDViewInformationWidget.h
  qDMMLThreeDViewInformationWidget_p.h
  qDMMLThreeDWidget.cxx
  qDMMLThreeDWidget.h
  qDMMLTransformSliders.cxx
  qDMMLTransformSliders.h
  qDMMLTreeView.cxx
  qDMMLTreeView.h
  qDMMLTreeView_p.h
  qDMMLUtils.cxx
  qDMMLUtils.h
  qDMMLViewControllerBar.cxx
  qDMMLViewControllerBar.h
  qDMMLViewControllerBar_p.h
  qDMMLVolumeInfoWidget.cxx
  qDMMLVolumeInfoWidget.h
  qDMMLVolumeThresholdWidget.cxx
  qDMMLVolumeThresholdWidget.h
  qDMMLVolumeWidget.cxx
  qDMMLVolumeWidget.h
  qDMMLWidget.cxx
  qDMMLWidget.h
  qDMMLWindowLevelWidget.cxx
  qDMMLWindowLevelWidget.h
  )

if(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  list(APPEND DMMLWidgets_SRCS
    qDMMLExpandingWebView.cxx
    qDMMLExpandingWebView.h
    qDMMLExpandingWebView_p.h
    )
endif()

# Headers that should run through moc
set(DMMLWidgets_MOC_SRCS
  qDMMLCaptureToolBar.h
  qDMMLCheckableNodeComboBox.h
  qDMMLClipNodeWidget.h
  qDMMLCollapsibleButton.h
  qDMMLColorListView.h
  qDMMLColorPickerWidget.h
  qDMMLColorTableComboBox.h
  qDMMLColorTableView.h
  qDMMLColorModel.h
  qDMMLCoordinatesWidget.h
  qDMMLDisplayNodeViewComboBox.h
  qDMMLDisplayNodeWidget.h
  qDMMLEventBrokerConnection.h
  qDMMLEventBrokerWidget.h
  qDMMLEventLogger.h
  qDMMLEventLoggerWidget.h
  qDMMLItemDelegate.h
  qDMMLLabelComboBox.h
  qDMMLLayoutManager.h
  qDMMLLayoutManager_p.h
  qDMMLLayoutViewFactory.h
  qDMMLLayoutWidget.h
  qDMMLLinearTransformSlider.h
  qDMMLListWidget.h
  qDMMLMatrixWidget.h
  qDMMLModelInfoWidget.h
  qDMMLNavigationView.h
  qDMMLNodeAttributeTableView.h
  qDMMLNodeAttributeTableWidget.h
  qDMMLNodeComboBox.h
  qDMMLNodeComboBoxDelegate.h
  qDMMLNodeComboBoxMenuDelegate.h
  qDMMLNodeFactory.h
  qDMMLPlotWidget.h
  qDMMLPlotViewControllerWidget.h
  qDMMLPlotViewControllerWidget_p.h
  qDMMLPlotView_p.h
  qDMMLPlotView.h
  qDMMLRangeWidget.h
  qDMMLROIWidget.h
  qDMMLScalarInvariantComboBox.h
  qDMMLScalarsDisplayWidget.h
  qDMMLSceneCategoryModel.h
  qDMMLSceneColorTableModel.h
  qDMMLSceneFactoryWidget.h
  qDMMLSceneModel.h
  qDMMLSceneViewMenu.h
  qDMMLSceneViewMenu_p.h
  qDMMLSceneTransformModel.h
  qDMMLSceneHierarchyModel.h
  qDMMLSceneDisplayableModel.h
  qDMMLScreenShotDialog.h
  qDMMLSegmentSelectorWidget.h
  qDMMLSliceControllerWidget.h
  qDMMLSliceControllerWidget_p.h
  qDMMLSliceInformationWidget.h
  qDMMLSliceInformationWidget_p.h
  qDMMLSliceView.h
  qDMMLSliceView_p.h
  qDMMLSliceWidget.h
  qDMMLSliceWidget_p.h
  qDMMLSliderWidget.h
  qDMMLSortFilterHierarchyProxyModel.h
  qDMMLSortFilterProxyModel.h
  qDMMLSpinBox.h
  qDMMLTableModel.h
  qDMMLTableView.h
  qDMMLTableView_p.h
  qDMMLTableViewControllerWidget.h
  qDMMLTableViewControllerWidget_p.h
  qDMMLTableWidget.h
  qDMMLThreeDView.h
  qDMMLThreeDView_p.h
  qDMMLThreeDViewControllerWidget.h
  qDMMLThreeDViewInformationWidget.h
  qDMMLThreeDViewInformationWidget_p.h
  qDMMLThreeDWidget.h
  qDMMLTransformSliders.h
  qDMMLTreeView.h
  qDMMLUtils.h
  qDMMLViewControllerBar.h
  qDMMLViewControllerBar_p.h
  qDMMLVolumeInfoWidget.h
  qDMMLVolumeThresholdWidget.h
  qDMMLVolumeWidget.h
  qDMMLVolumeWidget_p.h
  qDMMLWidget.h
  qDMMLWindowLevelWidget.h
  )

if(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  list(APPEND DMMLWidgets_MOC_SRCS
    qDMMLExpandingWebView.h
    qDMMLExpandingWebView_p.h
    )
endif()

# UI files
set(DMMLWidgets_UI_SRCS
  Resources/UI/qDMMLClipNodeWidget.ui
  Resources/UI/qDMMLColorPickerWidget.ui
  Resources/UI/qDMMLDisplayNodeWidget.ui
  Resources/UI/qDMMLEventLoggerWidget.ui
  Resources/UI/qDMMLModelInfoWidget.ui
  Resources/UI/qDMMLNodeAttributeTableView.ui
  Resources/UI/qDMMLNodeAttributeTableWidget.ui
  Resources/UI/qDMMLPlotViewControllerWidget.ui
  Resources/UI/qDMMLROIWidget.ui
  Resources/UI/qDMMLScalarsDisplayWidget.ui
  Resources/UI/qDMMLSceneFactoryWidget.ui
  Resources/UI/qDMMLScreenShotDialog.ui
  Resources/UI/qDMMLSegmentSelectorWidget.ui
  Resources/UI/qDMMLSliceControllerWidget.ui
  Resources/UI/qDMMLSliceInformationWidget.ui
  Resources/UI/qDMMLSliceWidget.ui
  Resources/UI/qDMMLTableViewControllerWidget.ui
  Resources/UI/qDMMLThreeDViewControllerWidget.ui
  Resources/UI/qDMMLThreeDViewInformationWidget.ui
  Resources/UI/qDMMLTransformSliders.ui
  Resources/UI/qDMMLVolumeInfoWidget.ui
  Resources/UI/qDMMLVolumeThresholdWidget.ui
  Resources/UI/qDMMLWindowLevelWidget.ui
  )

set(DMMLWidgets_QRC_SRCS
  Resources/qDMMLWidgets.qrc
  )

if(Cjyx_USE_QtTesting)
  list(APPEND DMMLWidgets_SRCS
    qDMMLCheckableNodeComboBoxEventPlayer.cpp
    qDMMLCheckableNodeComboBoxEventPlayer.h
    qDMMLNodeComboBoxEventPlayer.cpp
    qDMMLNodeComboBoxEventPlayer.h
    qDMMLNodeComboBoxEventTranslator.cpp
    qDMMLNodeComboBoxEventTranslator.h
    qDMMLTreeViewEventPlayer.cpp
    qDMMLTreeViewEventPlayer.h
    qDMMLTreeViewEventTranslator.cpp
    qDMMLTreeViewEventTranslator.h
    )

  list(APPEND DMMLWidgets_MOC_SRCS
    qDMMLCheckableNodeComboBoxEventPlayer.h
    qDMMLNodeComboBoxEventPlayer.h
    qDMMLNodeComboBoxEventTranslator.h
    qDMMLTreeViewEventPlayer.h
    qDMMLTreeViewEventTranslator.h
    )
endif()

  set(_moc_options OPTIONS -DDMML_WIDGETS_HAVE_QT5)
  if(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
    list(APPEND _moc_options -DDMML_WIDGETS_HAVE_WEBENGINE_SUPPORT)
  endif()
  QT5_WRAP_CPP(DMMLWidgets_MOC_CXX ${DMMLWidgets_MOC_SRCS} ${_moc_options})
  QT5_WRAP_UI(DMMLWidgets_UI_CXX ${DMMLWidgets_UI_SRCS})
  QT5_ADD_RESOURCES(DMMLWidgets_QRC_CXX ${DMMLWidgets_QRC_SRCS})

source_group("Resources" FILES
  ${DMMLWidgets_UI_SRCS}
  ${DMMLWidgets_QRC_SRCS}
  )

source_group("Generated" FILES
  ${DMMLWidgets_MOC_CXX}
  ${DMMLWidgets_UI_CXX}
  ${DMMLWidgets_QRC_CXX}
  )

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${DMMLCore_INCLUDE_DIRS}
  ${DMMLLogic_INCLUDE_DIRS}
  ${DMMLDisplayableManager_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

  # --------------------------------------------------------------------------
  # Translation
  # --------------------------------------------------------------------------
  if(Cjyx_BUILD_I18N_SUPPORT)
    set(TS_DIR
      "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Translations/"
    )

    include(CjyxMacroTranslation)
    CjyxMacroTranslation(
      SRCS ${DMMLWidgets_SRCS}
      UI_SRCS ${DMMLWidgets_UI_SRCS}
      TS_DIR ${TS_DIR}
      TS_BASEFILENAME ${PROJECT_NAME}
      TS_LANGUAGES ${Cjyx_LANGUAGES}
      QM_OUTPUT_DIR_VAR QM_OUTPUT_DIR
      QM_OUTPUT_FILES_VAR QM_OUTPUT_FILES
      )

    set_property(GLOBAL APPEND PROPERTY Cjyx_QM_OUTPUT_DIRS ${QM_OUTPUT_DIR})
  else()
    set(QM_OUTPUT_FILES )
  endif()

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(lib_name ${PROJECT_NAME})
set(targets_to_export ${lib_name})

add_library(${lib_name}
  ${DMMLWidgets_SRCS}
  ${DMMLWidgets_MOC_CXX}
  ${DMMLWidgets_UI_CXX}
  ${DMMLWidgets_QRC_CXX}
  ${QM_OUTPUT_FILES}
  )

set(DMMLWidgets_LIBRARIES
  ${QT_LIBRARIES}
  ${CTK_LIBRARIES}
  DMMLLogic
  DMMLDisplayableManager
  VTK::RenderingQt
  )

target_link_libraries(${lib_name} ${DMMLWidgets_LIBRARIES})

# Apply user-defined properties to the library target.
if(Cjyx_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Cjyx_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_FOLDER)
  set(${PROJECT_NAME}_FOLDER ${PROJECT_NAME})
endif()
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(${lib_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# PythonQt wrapping
# --------------------------------------------------------------------------
if(DMMLWidgets_WRAP_PYTHON)
  ctkMacroBuildLibWrapper(
    NAMESPACE "org.cjyx.libs"
    TARGET ${lib_name}
    SRCS "${DMMLWidgets_SRCS}"
    INSTALL_BIN_DIR ${Cjyx_INSTALL_BIN_DIR}
    INSTALL_LIB_DIR ${Cjyx_INSTALL_LIB_DIR}
    )
  # Folder
  if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name}PythonQt PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
  endif()
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${targets_to_export} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Qt Designer Plugins
# --------------------------------------------------------------------------
if(DMML_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
