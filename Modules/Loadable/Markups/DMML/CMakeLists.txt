project(vtkCjyx${MODULE_NAME}ModuleDMML)

find_package(RapidJSON REQUIRED)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${RapidJSON_INCLUDE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleVTKWidgets_SOURCE_DIR}
  ${vtkCjyx${MODULE_NAME}ModuleVTKWidgets_BINARY_DIR}
  )

set(${KIT}_SRCS
  vtkDMML${MODULE_NAME}AngleNode.cxx
  vtkDMML${MODULE_NAME}ClosedCurveNode.cxx
  vtkDMML${MODULE_NAME}CurveNode.cxx
  vtkDMML${MODULE_NAME}DisplayNode.cxx
  vtkDMML${MODULE_NAME}FiducialDisplayNode.cxx
  vtkDMML${MODULE_NAME}FiducialStorageNode.cxx
  vtkDMML${MODULE_NAME}FiducialNode.cxx
  vtkDMML${MODULE_NAME}JsonStorageNode.cxx
  vtkDMML${MODULE_NAME}LineNode.cxx
  vtkDMML${MODULE_NAME}Node.cxx
  vtkDMML${MODULE_NAME}PlaneDisplayNode.cxx
  vtkDMML${MODULE_NAME}PlaneJsonStorageNode.cxx
  vtkDMML${MODULE_NAME}PlaneNode.cxx
  vtkDMML${MODULE_NAME}ROIDisplayNode.cxx
  vtkDMML${MODULE_NAME}ROIJsonStorageNode.cxx
  vtkDMML${MODULE_NAME}ROINode.cxx
  vtkCurveGenerator.cxx
  vtkCurveGenerator.h
  vtkCurveMeasurementsCalculator.cxx
  vtkCurveMeasurementsCalculator.h
  vtkLinearSpline.cxx
  vtkLinearSpline.h
  vtkDMMLMeasurementAngle.cxx
  vtkDMMLMeasurementAngle.h
  vtkDMMLMeasurementArea.cxx
  vtkDMMLMeasurementArea.h
  vtkDMMLMeasurementLength.cxx
  vtkDMMLMeasurementLength.h
  vtkDMMLMeasurementVolume.cxx
  vtkDMMLMeasurementVolume.h
  vtkParametricPolynomialApproximation.cxx
  vtkParametricPolynomialApproximation.h
  vtkProjectMarkupsCurvePointsFilter.cxx
  vtkProjectMarkupsCurvePointsFilter.h
  vtkCjyxDijkstraGraphGeodesicPath.cxx
  vtkCjyxDijkstraGraphGeodesicPath.h
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
