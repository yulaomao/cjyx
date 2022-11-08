project(vtkCjyx${MODULE_NAME}ModuleDMML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMML_EXPORT")

include_directories(BEFORE ${vtkTeem_INCLUDE_DIRS})

set(${KIT}_INCLUDE_DIRECTORIES
  ${Cjyx_Base_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkDMMLSequenceBrowserNode.cxx
  vtkDMMLSequenceBrowserNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${DMML_LIBRARIES}
  CjyxBaseLogic
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleDMML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
