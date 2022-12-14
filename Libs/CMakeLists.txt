project(CjyxLibs)

# Enable this option to avoid unnecessary re-compilation associated with command line module
set(GENERATECLP_USE_MD5 ON)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------

set(dirs )

list(APPEND dirs
  ITKFactoryRegistration
  )
set(CjyxExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES
  ${CjyxExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES} ITKFactoryRegistration
  CACHE INTERNAL "CjyxExecutionModel extra executable target libraries" FORCE
  )

if(Cjyx_BUILD_vtkAddon)
  list(APPEND dirs
    vtkAddon
    )
endif()

list(APPEND dirs
  vtkTeem
  vtkITK
  vtkSegmentationCore
  DMML/Core
  )
if(Cjyx_BUILD_CLI_SUPPORT)
  list(APPEND dirs DMML/CLI)
endif()
list(APPEND dirs
  RemoteIO
  DMML/Logic
  DMML/DisplayableManager
  )
if(Cjyx_BUILD_CLI_SUPPORT)
  list(APPEND dirs DMML/IDImageIO)
endif()
list(APPEND dirs
  DMML/Widgets
  )

#-----------------------------------------------------------------------------
# Set library properties
#-----------------------------------------------------------------------------

if(Cjyx_BUILD_CLI_SUPPORT)
  # Python support within ModuleDescriptionParser is used only within ModuleFactory.cxx
  # Considering the ModuleFactory isn't used anymore to discover modules, let's disable the
  # associated python support so that the python library aren't a dependency of
  # either ModuleDescriptionParser or program/library depending on ModuleDescriptionParser.
  set(ModuleDescriptionParser_USE_PYTHON OFF)
endif()

set(DMML_APPLICATION_NAME "${Cjyx_MAIN_PROJECT_APPLICATION_NAME}" CACHE STRING "Current DMML scene creator application name" FORCE)

# Convert version to 0xXXYYZZ format where XX=major, YY=minor, ZZ=patch version in hexadecimal.
# Add a leading 1 (256*256*256*1) to force having a leading 0 for major version (e.g., 04 instead of just 4)
math(EXPR DMML_APPLICATION_VERSION_HEX "1 << 24
  | ${Cjyx_MAIN_PROJECT_VERSION_MAJOR} << 16
  | ${Cjyx_MAIN_PROJECT_VERSION_MINOR} << 8
  | ${Cjyx_MAIN_PROJECT_VERSION_PATCH}"
  OUTPUT_FORMAT HEXADECIMAL)

# Remove leading 1 (remove 0x1 and add back just 0x)
string(SUBSTRING "${DMML_APPLICATION_VERSION_HEX}" 3 6 DMML_APPLICATION_VERSION_HEX)
set(DMML_APPLICATION_VERSION "0x${DMML_APPLICATION_VERSION_HEX}" CACHE STRING "Current DMML application version" FORCE)

set(DMML_APPLICATION_REVISION "${Cjyx_MAIN_PROJECT_REVISION}" CACHE STRING "Current DMML application revision number" FORCE)

# DMML supports Cjyx3 and Cjyx4 scenes
set(DMML_APPLICATION_SUPPORT_VERSION "0x030000" CACHE STRING "DMML application version to support" FORCE)

message(STATUS "Configuring DMML")
message(STATUS "  DMML_APPLICATION_NAME is ${DMML_APPLICATION_NAME}")
message(STATUS "  DMML_APPLICATION_VERSION is ${DMML_APPLICATION_VERSION}")
message(STATUS "  DMML_APPLICATION_REVISION is ${DMML_APPLICATION_REVISION}")
message(STATUS "  DMML_APPLICATION_SUPPORT_VERSION is ${DMML_APPLICATION_SUPPORT_VERSION}")

# Qt designer plugins
set(DMML_BUILD_QT_DESIGNER_PLUGINS ${Cjyx_BUILD_QT_DESIGNER_PLUGINS})

# ITKFactories directories
set(DMMLIDImageIO_ITKFACTORIES_DIR ${Cjyx_ITKFACTORIES_DIR})
set(DMMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${Cjyx_INSTALL_ITKFACTORIES_DIR})

# Name of the environment variable that contains the application home directory.
set(DMML_APPLICATION_HOME_DIR_ENV "CJYX_HOME")

# Name of the environment variable that contains the requested OpenGL profile.
# Accepted values are "default" (same as not specifying a value) or
# to force a specific profile: "no", "core", or  "compatibility".
set(DMML_APPLICATION_OPENGL_PROFILE_ENV "CJYX_OPENGL_PROFILE")

# Name of the subdirectory within the application directory where shared files are stored.
set(DMML_APPLICATION_SHARE_SUBDIR ${Cjyx_SHARE_DIR})

# vtkITK contains tests that uses DMML's test data.
set(DMML_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DMML/Core/Testing/TestData)

set(VTKITK_BUILD_DICOM_SUPPORT ${Cjyx_BUILD_DICOM_SUPPORT})

set(DMML_WIDGETS_HAVE_WEBENGINE_SUPPORT ${Cjyx_BUILD_WEBENGINE_SUPPORT})

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  string(REPLACE "/" "" dir_noslash ${dir})
  if(dir_noslash MATCHES "Widgets$")
    set(dir_noslash "q${dir_noslash}")
  endif()
  set(${dir_noslash}_INSTALL_BIN_DIR ${Cjyx_INSTALL_LIB_DIR})
  set(${dir_noslash}_INSTALL_LIB_DIR ${Cjyx_INSTALL_LIB_DIR})
  set(${dir_noslash}_INSTALL_SHARE_DIR ${Cjyx_INSTALL_ROOT}share/${dir_noslash})
  set(${dir_noslash}_INSTALL_NO_DEVELOPMENT ${Cjyx_INSTALL_NO_DEVELOPMENT})
  set(${dir_noslash}_EXPORT_FILE ${Cjyx_BINARY_DIR}/CjyxTargets.cmake)
  set(${dir_noslash}_FOLDER Core-Libs/${dir_noslash})

  # Check if library sources were specified as a remote module
  if(DEFINED ${dir}_SOURCE_DIR AND Cjyx_BUILD_${dir})
    message(STATUS "Configuring library: ${dir} [${dir}_SOURCE_DIR: ${${dir}_SOURCE_DIR}]")
    add_subdirectory(
      ${${dir}_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/${dir}
      )
  else()
    message(STATUS "Configuring library: ${dir}")
    add_subdirectory(${dir})
  endif()

endforeach()

#-----------------------------------------------------------------------------
# Generate list of include directories
#-----------------------------------------------------------------------------
set(include_dirs)
foreach(dir ${dirs})
  string(REPLACE "/" "" dir_noslash ${dir})
  if(dir_noslash MATCHES "Widgets$")
    set(dir_noslash "q${dir_noslash}")
  endif()
  list(APPEND include_dirs ${${dir_noslash}_INCLUDE_DIRS})
endforeach()

# Set variable Cjyx_Libs_INCLUDE_DIRS
set(Cjyx_Libs_INCLUDE_DIRS ${include_dirs} CACHE INTERNAL "Cjyx Libs includes" FORCE)

#-----------------------------------------------------------------------------
# Set DMML_LIBRARIES
#-----------------------------------------------------------------------------
# Anticipating the move of DMML related libraries to a standalone project,
# let's group these libraries.
# Note also that since DMMLIDImageIO is an ITK plugin it is NOT added to that list.
set(_dmml_libraries DMMLCore DMMLLogic DMMLDisplayableManager)
if(Cjyx_BUILD_CLI_SUPPORT)
  list(APPEND _dmml_libraries DMMLCLI)
endif()
set(DMML_LIBRARIES ${_dmml_libraries} CACHE INTERNAL "DMML libraries" FORCE)

#-----------------------------------------------------------------------------
# Set variable Cjyx_Libs_VTK_WRAPPED_LIBRARIES
#-----------------------------------------------------------------------------
set(_vtk_wrapped_libraries
  vtkSegmentationCore
  vtkTeem
  vtkITK
  DMMLCore
  DMMLLogic
  )

if(Cjyx_BUILD_vtkAddon)
  list(APPEND _vtk_wrapped_libraries
    vtkAddon
    )
endif()
if(Cjyx_USE_PYTHONQT AND VTK_WRAP_PYTHON)
  list(APPEND _vtk_wrapped_libraries DMMLDisplayableManager)
endif()
if(Cjyx_BUILD_CLI_SUPPORT)
  list(APPEND _vtk_wrapped_libraries DMMLCLI)
endif()
set(Cjyx_Libs_VTK_WRAPPED_LIBRARIES
  ${_vtk_wrapped_libraries} CACHE INTERNAL "Cjyx Libs VTK wrapped" FORCE)
