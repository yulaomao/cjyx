
#-----------------------------------------------------------------------------
set(MODULE_NAME ExecutionModelTour)

#-----------------------------------------------------------------------------

#
# CjyxExecutionModel
#
find_package(CjyxExecutionModel REQUIRED)
include(${CjyxExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Cjyx_SOURCE_DIR}/Resources/BillsLogo.h
  TARGET_LIBRARIES
    # ${VTK_LIBRARIES} # Not needed: All dependencies are transitively satisfied by other targets
    DMMLCore
    vtkCjyxMarkupsModuleDMML
  INCLUDE_DIRECTORIES
    ${DMMLCore_INCLUDE_DIRS}
    ${vtkCjyxMarkupsModuleDMML_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
