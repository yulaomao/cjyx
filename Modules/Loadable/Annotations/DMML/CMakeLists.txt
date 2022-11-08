project(vtkCjyx${MODULE_NAME}ModuleDMML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  vtkDMMLAnnotationSnapshotNode.cxx
  vtkDMMLAnnotationSnapshotNode.h
  vtkDMMLAnnotationSnapshotStorageNode.cxx
  vtkDMMLAnnotationSnapshotStorageNode.h
  vtkDMMLAnnotationHierarchyNode.cxx
  vtkDMMLAnnotationHierarchyNode.h
  vtkDMMLAnnotationNode.cxx
  vtkDMMLAnnotationNode.h
  vtkDMMLAnnotationStorageNode.cxx
  vtkDMMLAnnotationStorageNode.h
  vtkDMMLAnnotationDisplayNode.cxx
  vtkDMMLAnnotationDisplayNode.h
  vtkDMMLAnnotationTextDisplayNode.cxx
  vtkDMMLAnnotationTextDisplayNode.h
  vtkDMMLAnnotationControlPointsNode.cxx
  vtkDMMLAnnotationControlPointsNode.h
  vtkDMMLAnnotationControlPointsStorageNode.cxx
  vtkDMMLAnnotationControlPointsStorageNode.h
  vtkDMMLAnnotationPointDisplayNode.cxx
  vtkDMMLAnnotationPointDisplayNode.h
  vtkDMMLAnnotationLinesNode.cxx
  vtkDMMLAnnotationLinesNode.h
  vtkDMMLAnnotationLinesStorageNode.cxx
  vtkDMMLAnnotationLinesStorageNode.h
  vtkDMMLAnnotationLineDisplayNode.cxx
  vtkDMMLAnnotationLineDisplayNode.h
  vtkDMMLAnnotationRulerNode.cxx
  vtkDMMLAnnotationRulerNode.h
  vtkDMMLAnnotationRulerStorageNode.cxx
  vtkDMMLAnnotationRulerStorageNode.h
  vtkDMMLAnnotationFiducialNode.cxx
  vtkDMMLAnnotationFiducialNode.h
  vtkDMMLAnnotationFiducialsStorageNode.cxx
  vtkDMMLAnnotationFiducialsStorageNode.h
  vtkDMMLAnnotationTextNode.cxx
  vtkDMMLAnnotationTextNode.h
  vtkDMMLAnnotationROINode.cxx
  vtkDMMLAnnotationROINode.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${DMML_LIBRARIES}
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleDMML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
