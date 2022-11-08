project(DMMLCore)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)
#-----------------------------------------------------------------------------

# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

set(DMML_APPLICATION_NAME "Cjyx" CACHE STRING "Current DMML scene creator application name" )
mark_as_advanced(DMML_APPLICATION_NAME)

# The format is: 0xXXYYZZ where XX is the major version, YY the minor and
# ZZ the patch version.
# This format is chosen to allow easy comparison in C++ precompiler macros.
set(DMML_APPLICATION_VERSION 0x040000 CACHE STRING "Current DMML application version" )
mark_as_advanced(DMML_APPLICATION_VERSION)

# Revision can be used to identify version with finer granularity than version.
# It is an integer number that is incremented with each new software revision.
set(DMML_APPLICATION_REVISION 0 CACHE STRING "Current DMML application revision number" )
mark_as_advanced(DMML_APPLICATION_REVISION)

# DMML scenes that are older than DMML_APPLICATION_SUPPORT_VERSION won't be supported.
# Or said differently, DMML_APPLICATION_SUPPORT_VERSION is the earliest version supported
# for a DMML scene.
# The format is: 0xXXYYZZ where XX is the major version, YY the minor and
# ZZ the patch version.
# For example, set DMML_APPLICATION_SUPPORT_VERSION to 0x040000 if you don't want to
# support scenes of Cjyx 3.
# Supports Cjyx3 and Cjyx4 scenes by default.
set(DMML_APPLICATION_SUPPORT_VERSION 0x030000 CACHE STRING "DMML application version to support" )
mark_as_advanced(DMML_APPLICATION_SUPPORT_VERSION)

option(DMML_USE_vtkTeem "Build DMML with vtkTeem support." ON)
mark_as_advanced(DMML_USE_vtkTeem)

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# VTK
#
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKCommon
  ITKIOImageBase
  ITKIOSpatialObjects
  ITKIOTransformBase
  ITKMesh
  ITKTransform
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
if(ITK_VERSION VERSION_GREATER_EQUAL "5.3")
  foreach(factory_uc IN ITEMS "IMAGEIO" "MESHIO" "TRANSFORMIO")
    set(ITK_NO_${factory_uc}_FACTORY_REGISTER_MANAGER 1)
  endforeach()
else()
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
endif()
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS ${ITKFactoryRegistration_INCLUDE_DIRS})
include(${ITK_USE_FILE})

#
# LibArchive
#
find_package(LibArchive REQUIRED MODULE)

#
# vtkTeem
#
if(DMML_USE_vtkTeem)
  find_package(Teem REQUIRED)
  include(${Teem_USE_FILE})
endif()

#
# vtkAddon
#
if(NOT Cjyx_BUILD_vtkAddon)
  find_package(vtkAddon REQUIRED)
endif()

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkAddon_INCLUDE_DIRS}
  ${vtkITK_INCLUDE_DIRS}
  ${vtkSegmentationCore_INCLUDE_DIRS}
  ${LibArchive_INCLUDE_DIR}
  )
if(DMML_USE_vtkTeem)
  list(APPEND include_dirs ${vtkTeem_INCLUDE_DIRS})
endif()
include_directories(BEFORE ${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file vtkDMMLConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(DMMLCore_SRCS
  vtkArchive.cxx
  vtkArchive.h
  vtkCodedEntry.cxx
  vtkEventBroker.cxx
  vtkDataFileFormatHelper.cxx
  vtkDMMLMeasurement.cxx
  vtkDMMLStaticMeasurement.cxx
  vtkDMMLLogic.cxx
  vtkDMMLAbstractLayoutNode.cxx
  vtkDMMLAbstractViewNode.cxx
  vtkDMMLCameraNode.cxx
  vtkDMMLClipModelsNode.cxx
  vtkDMMLColorNode.cxx
  vtkDMMLColors.cxx
  vtkDMMLColorTableNode.cxx
  vtkDMMLColorTableStorageNode.cxx
  vtkDMMLCoreTestingUtilities.cxx
  vtkDMMLCrosshairNode.cxx
  vtkDMMLDiffusionTensorDisplayPropertiesNode.cxx
  vtkDMMLDiffusionWeightedVolumeDisplayNode.cxx
  vtkDMMLDiffusionWeightedVolumeNode.cxx
  vtkDMMLDisplayNode.cxx
  vtkDMMLDisplayableNode.cxx
  vtkDMMLFolderDisplayNode.cxx
  vtkDMMLHierarchyNode.cxx
  vtkDMMLHierarchyStorageNode.cxx
  vtkDMMLDisplayableHierarchyNode.cxx
  vtkDMMLInteractionNode.cxx
  vtkDMMLLabelMapVolumeDisplayNode.cxx
  vtkDMMLLabelMapVolumeNode.cxx
  vtkDMMLLinearTransformNode.cxx
  vtkDMMLLinearTransformSequenceStorageNode.cxx
  vtkDMMLLinearTransformSequenceStorageNode.h
  vtkDMMLMarkupsStorageNode.cxx
  vtkDMMLMessageCollection.cxx
  vtkDMMLModelDisplayNode.cxx
  vtkDMMLModelHierarchyNode.cxx
  vtkDMMLModelNode.cxx
  vtkDMMLModelStorageNode.cxx
  vtkDMMLNode.cxx
  vtkDMMLParser.cxx
  vtkDMMLPlotChartNode.cxx
  vtkDMMLPlotSeriesNode.cxx
  vtkDMMLPlotViewNode.cxx
  vtkDMMLProceduralColorNode.cxx
  vtkDMMLProceduralColorStorageNode.cxx
  vtkDMMLROIListNode.cxx
  vtkDMMLROINode.cxx
  vtkDMMLScalarVolumeDisplayNode.cxx
  vtkDMMLScalarVolumeNode.cxx
  vtkDMMLScene.cxx
  vtkDMMLSceneViewNode.cxx
  vtkDMMLSceneViewStorageNode.cxx
  vtkDMMLScriptedModuleNode.cxx
  vtkDMMLScriptedModuleNode.h
  vtkDMMLSegmentationNode.cxx
  vtkDMMLSegmentationNode.h
  vtkDMMLSegmentationDisplayNode.cxx
  vtkDMMLSegmentationDisplayNode.h
  vtkDMMLSegmentationStorageNode.cxx
  vtkDMMLSegmentationStorageNode.h
  vtkDMMLSequenceNode.cxx
  vtkDMMLSequenceNode.h
  vtkDMMLSequenceStorageNode.cxx
  vtkDMMLSequenceStorageNode.h
  vtkDMMLSelectionNode.cxx
  vtkDMMLSliceCompositeNode.cxx
  vtkDMMLSliceDisplayNode.cxx
  vtkDMMLSliceNode.cxx
  vtkDMMLSnapshotClipNode.cxx
  vtkDMMLStorableNode.cxx
  vtkDMMLStorageNode.cxx
  vtkDMMLSubjectHierarchyConstants.h
  vtkDMMLSubjectHierarchyNode.cxx
  vtkDMMLSubjectHierarchyNode.h
  vtkDMMLSubjectHierarchyLegacyNode.cxx
  vtkDMMLSubjectHierarchyLegacyNode.h
  vtkDMMLTableNode.cxx
  vtkDMMLTableStorageNode.cxx
  vtkDMMLTableSQLiteStorageNode.cxx
  vtkDMMLTableViewNode.cxx
  vtkDMMLTextNode.cxx
  vtkDMMLTextStorageNode.cxx
  vtkDMMLTransformNode.cxx
  vtkDMMLTransformStorageNode.cxx
  vtkDMMLTransformDisplayNode.cxx
  vtkDMMLTransformableNode.cxx
  vtkDMMLUnitNode.cxx
  vtkDMMLVectorVolumeDisplayNode.cxx
  vtkDMMLViewNode.cxx
  vtkDMMLVolumeArchetypeStorageNode.cxx
  vtkDMMLVolumeDisplayNode.cxx
  vtkDMMLGlyphableVolumeDisplayNode.cxx
  vtkDMMLGlyphableVolumeSliceDisplayNode.cxx
  vtkDMMLVolumeHeaderlessStorageNode.cxx
  vtkDMMLVolumeNode.cxx
  vtkDMMLVolumeSequenceStorageNode.cxx
  vtkDMMLVolumeSequenceStorageNode.h
  vtkObservation.cxx
  vtkObserverManager.cxx
  vtkDMMLLayoutNode.cxx
  # Classes for remote data handling:
  vtkCacheManager.cxx
  vtkDataIOManager.cxx
  vtkDataTransfer.cxx
  vtkDMMLBSplineTransformNode.cxx
  vtkDMMLGridTransformNode.cxx
  vtkPermissionPrompter.cxx
  vtkURIHandler.cxx
  vtkTagTableCollection.cxx
  vtkTagTable.cxx
  vtkDMMLdGEMRICProceduralColorNode.cxx
  vtkDMMLPETProceduralColorNode.cxx
  # Note: when you add new node types, add
  # register code to the vtkDMMLScene constructor
  )

if(DMML_USE_vtkTeem)
  set(DMMLCore_SRCS
    ${DMMLCore_SRCS}
    vtkDMMLDiffusionImageVolumeNode.cxx
    vtkDMMLDiffusionTensorVolumeDisplayNode.cxx
    vtkDMMLDiffusionTensorVolumeNode.cxx
    vtkDMMLDiffusionTensorVolumeSliceDisplayNode.cxx
    vtkDMMLNRRDStorageNode.cxx
    vtkDMMLTensorVolumeNode.cxx
    vtkDMMLVectorVolumeNode.cxx
    vtkDMMLStreamingVolumeNode.cxx
    )
endif()

set_source_files_properties(
  vtkDMMLCoreTestingUtilities.cxx
  WRAP_EXCLUDE
  )

# Abstract/pure virtual classes

set_source_files_properties(
  vtkDMMLNode.cxx
  vtkDMMLVolumeNode.cxx
  vtkDMMLTransformNode.cxx
  vtkDMMLDisplayNode.cxx
  vtkDMMLDisplayableNode.cxx
  vtkDMMLVolumeDisplayNode.cxx
  ABSTRACT
  )

string(REGEX REPLACE "\\.cxx" ".h" DMMLCore_SRCS_HEADERS "${DMMLCore_SRCS}")
source_group("Header Files" FILES ${DMMLCore_SRCS_HEADERS})

#-----------------------------------------------------------------------------
# Wrapping hints used by all Tcl, Python and Java wrapping.
# It can also be used by external wrapping tools.
set(VTK_WRAP_HINTS ${CMAKE_CURRENT_SOURCE_DIR}/Wrapping/Tcl/hints)
# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
set(srcs ${DMMLCore_SRCS} ${DMMLCore_SRCS_HEADERS})

set(lib_name ${PROJECT_NAME})
add_library(${lib_name} ${srcs})

set(libs
  vtkAddon
  vtkITK
  vtkSegmentationCore
  ${ITK_LIBRARIES}
  ${VTK_LIBRARIES}
  VTK::IOInfovis
  ${LibArchive_LIBRARY}
  )
if(DMML_USE_vtkTeem)
  list(APPEND libs vtkTeem)
endif()
target_link_libraries(${lib_name} ${libs})

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
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

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
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${lib_name}
    KIT_SRCS ${DMMLCore_SRCS}
    KIT_INSTALL_BIN_DIR ${${PROJECT_NAME}_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${${PROJECT_NAME}_INSTALL_LIB_DIR}
    )
  # Export target
  export(TARGETS ${lib_name}Python APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})
  # Folder
  if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name}Python PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    if(TARGET ${lib_name}Hierarchy)
      set_target_properties(${lib_name}Hierarchy PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
    endif()
  endif()
endif()

# --------------------------------------------------------------------------
# Testing (requires some of the examples)
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

# --------------------------------------------------------------------------
# Install Test Data
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  #file(GLOB_RECURSE DMMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.dmml")
  #file(GLOB_RECURSE TCLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.tcl")
  #file(GLOB_RECURSE XMLFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.xml")
  #file(GLOB_RECURSE GZFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.gz")
  #file(GLOB_RECURSE NHDRFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nhdr")
  #file(GLOB_RECURSE NRRDFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.nrrd")
  #file(GLOB_RECURSE VTKFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Testing/*.vtk")

  #foreach(file
  #    ${DMMLFILES}
  #    ${TCLFILES}
  #    ${XMLFILES}
  #    ${GZFILES}
  #    ${NHDRFILES}
  #    ${NRRDFILES}
  #    ${VTKFILES}
  #    )
  #  message(file:${file})
  #  configure_file(
  #    ${CMAKE_CURRENT_SOURCE_DIR}/${file}
  #    ${CMAKE_BINARY_DIR}/share/${PROJECT_NAME}/${file}
  #    COPYONLY
  #    )
  #endforeach()

  install(DIRECTORY
    Testing
    DESTINATION share/${PROJECT_NAME} COMPONENT Development
    PATTERN "*.cxx" EXCLUDE
    PATTERN ".svn" EXCLUDE
    PATTERN "*.tcl" EXCLUDE
    PATTERN "CMakeLists.txt" EXCLUDE
    )
endif()

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${vtkAddon_INCLUDE_DIRS} ${vtkSegmentationCore_INCLUDE_DIRS}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
