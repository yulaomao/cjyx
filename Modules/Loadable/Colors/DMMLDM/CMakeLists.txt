project(vtkCjyx${MODULE_NAME}ModuleDMMLDisplayableManager)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMMLDISPLAYABLEMANAGER_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkCjyx${MODULE_NAME}ModuleLogic_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleLogic_BINARY_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleDMML_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleDMML_BINARY_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleVTKWidgets_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleVTKWidgets_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(displayable_manager_SRCS
  vtkDMMLColorLegendDisplayableManager.cxx
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
  vtkCjyx${MODULE_NAME}ModuleLogic
  vtkCjyx${MODULE_NAME}ModuleDMML
  vtkCjyx${MODULE_NAME}ModuleVTKWidgets
)

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
)
