project(vtkCjyx${MODULE_NAME}ModuleDMML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtkDMMLMarkupsTestLineNode.h
  vtkDMMLMarkupsTestLineNode.cxx
  )

set(${KIT}_TARGET_LIBRARIES
  ${DMML_LIBRARIES}
  vtkCjyxMarkupsModuleDMML
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleDMML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
