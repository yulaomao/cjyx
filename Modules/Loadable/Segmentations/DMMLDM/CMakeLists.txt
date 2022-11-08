project(vtkCjyx${MODULE_NAME}ModuleDMMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkCjyxSegmentationsModuleDMML_INCLUDE_DIRS}
  )

set(displayable_manager_SRCS
  vtkDMML${MODULE_NAME}DisplayableManager3D.cxx
  vtkDMML${MODULE_NAME}DisplayableManager2D.cxx
  )

CjyxConfigureDisplayableManagerObjectFactory(
  TARGET_NAME ${KIT}
  SRCS "${displayable_manager_SRCS}"
  EXPORT_MACRO "${${KIT}_EXPORT_DIRECTIVE}"
  EXPORT_HEADER "${KIT}Export.h"
  OUTPUT_SRCS_VAR displayable_manager_instantiator_SRCS
  )

set(${KIT}_SRCS
  ${displayable_manager_instantiator_SRCS}
  ${displayable_manager_SRCS}
  )

set(${KIT}_TARGET_LIBRARIES
  ${DMML_LIBRARIES}
  vtkCjyxSegmentationsModuleDMML
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
