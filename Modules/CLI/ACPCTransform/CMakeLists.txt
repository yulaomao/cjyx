
#-----------------------------------------------------------------------------
set(MODULE_NAME ACPCTransform)

#-----------------------------------------------------------------------------

#
# CjyxExecutionModel
#
find_package(CjyxExecutionModel REQUIRED)
include(${CjyxExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Cjyx_SOURCE_DIR}/Resources/NAMICLogo.h
  ADDITIONAL_SRCS
    vtkPrincipalAxesAlign.cxx
  TARGET_LIBRARIES
    ModuleDescriptionParser
    ${ITK_LIBRARIES}
    vtkTeem
    DMMLCore
    CjyxBaseCLI
    ${VTK_LIBRARIES}
    vtkCjyxMarkupsModuleDMML
  INCLUDE_DIRECTORIES
    ${vtkTeem_INCLUDE_DIRS}
    ${DMMLCore_INCLUDE_DIRS}
    ${vtkITK_INCLUDE_DIRS}
    ${CjyxBaseCLI_SOURCE_DIR}
    ${CjyxBaseCLI_BINARY_DIR}
    ${vtkCjyxMarkupsModuleDMML_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
