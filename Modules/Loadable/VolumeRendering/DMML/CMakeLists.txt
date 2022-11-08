project(vtkCjyx${MODULE_NAME}ModuleDMML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMML_EXPORT")

find_package(JsonCpp REQUIRED)

set(${KIT}_INCLUDE_DIRECTORIES
  ${JsonCpp_INCLUDE_DIR}
  ${vtkCjyxMarkupsModuleDMML_SOURCE_DIR}
  )

set(${KIT}_SRCS
  vtkDMMLCPURayCast${MODULE_NAME}DisplayNode.cxx
  vtkDMMLCPURayCast${MODULE_NAME}DisplayNode.h
  vtkDMMLGPURayCast${MODULE_NAME}DisplayNode.cxx
  vtkDMMLGPURayCast${MODULE_NAME}DisplayNode.h
  vtkDMMLMulti${MODULE_NAME}DisplayNode.cxx
  vtkDMMLMulti${MODULE_NAME}DisplayNode.h
  vtkDMMLShaderPropertyNode.cxx
  vtkDMMLShaderPropertyNode.h
  vtkDMMLShaderPropertyStorageNode.cxx
  vtkDMMLShaderPropertyStorageNode.h
  vtkDMMLVolumePropertyNode.cxx
  vtkDMMLVolumePropertyNode.h
  vtkDMMLVolumePropertyStorageNode.cxx
  vtkDMMLVolumePropertyStorageNode.h
  vtkDMML${MODULE_NAME}DisplayNode.cxx
  vtkDMML${MODULE_NAME}DisplayNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${JsonCpp_LIBRARY}
  ${DMML_LIBRARIES}
  vtkCjyxAnnotationsModuleDMML
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
