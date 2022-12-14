
#-----------------------------------------------------------------------------
set(MODULE_NAME GrayscaleModelMaker)

#-----------------------------------------------------------------------------

#
# CjyxExecutionModel
#
find_package(CjyxExecutionModel REQUIRED)
include(${CjyxExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
set(${MODULE_NAME}_TARGET_LIBRARIES
  vtkITK ModuleDescriptionParser DMMLCore CjyxBaseCLI ${VTK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Cjyx_SOURCE_DIR}/Resources/NAMICLogo.h
  TARGET_LIBRARIES ${${MODULE_NAME}_TARGET_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${DMMLCore_INCLUDE_DIRS}
  EXECUTABLE_ONLY
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  if(NOT WIN32)
    add_subdirectory(Testing)
  endif()
endif()
