project(DMMLIDImageIO)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16.3...3.19.7 FATAL_ERROR)
#-----------------------------------------------------------------------------


# --------------------------------------------------------------------------
# Options
# --------------------------------------------------------------------------
if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build with shared libraries." ON)
endif()

# --------------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------------

#
# ITK
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ITKCommon
  ITKIOImageBase
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
if(ITK_VERSION VERSION_GREATER_EQUAL "5.3")
  foreach(factory_uc IN ITEMS "IMAGEIO" "MESHIO" "TRANSFORMIO")
    set(ITK_NO_${factory_uc}_FACTORY_REGISTER_MANAGER 1)
  endforeach()
else()
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
endif()
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS ${ITKFactoryRegistration_INCLUDE_DIRS})
include(${ITK_USE_FILE})

#
# VTK
#
#find_package(VTK REQUIRED)
#include(${VTK_USE_FILE})

# --------------------------------------------------------------------------
# Include dirs
# --------------------------------------------------------------------------
set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${DMMLCore_INCLUDE_DIRS}
  )
include_directories(${include_dirs})

# --------------------------------------------------------------------------
# Configure headers
# --------------------------------------------------------------------------
set(configure_header_file itkDMMLIDImageIOConfigure.h)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/${configure_header_file}.in
  ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
  )

# --------------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  set(${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT ON)
endif()
if(NOT ${PROJECT_NAME}_INSTALL_NO_DEVELOPMENT)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
  install(
    FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/${configure_header_file}
    DESTINATION include/${PROJECT_NAME} COMPONENT Development)
endif()

# --------------------------------------------------------------------------
# Sources
# --------------------------------------------------------------------------
set(DMMLIDImageIO_SRCS
  itkDMMLIDImageIO.cxx
  itkDMMLIDImageIOFactory.cxx
  )

# --------------------------------------------------------------------------
# Build library
# --------------------------------------------------------------------------
# Note: Library name is different from the directory name !
set(lib_name DMMLIDIO)

set(srcs ${DMMLIDImageIO_SRCS})
add_library(${lib_name} ${srcs})

set(libs DMMLCore)
target_link_libraries(${lib_name} ${libs})

# Apply user-defined properties to the library target.
if(Cjyx_LIBRARY_PROPERTIES)
  set_target_properties(${lib_name} PROPERTIES ${Cjyx_LIBRARY_PROPERTIES})
endif()

# --------------------------------------------------------------------------
# Folder
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_FOLDER)
  set(${PROJECT_NAME}_FOLDER ${PROJECT_NAME})
endif()
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(${lib_name} PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# Export target
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_EXPORT_FILE)
  set(${PROJECT_NAME}_EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake)
endif()
export(TARGETS ${lib_name} APPEND FILE ${${PROJECT_NAME}_EXPORT_FILE})

# --------------------------------------------------------------------------
# Install library
# --------------------------------------------------------------------------
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_BIN_DIR)
  set(${PROJECT_NAME}_INSTALL_BIN_DIR bin)
endif()
if(NOT DEFINED ${PROJECT_NAME}_INSTALL_LIB_DIR)
  set(${PROJECT_NAME}_INSTALL_LIB_DIR lib/${PROJECT_NAME})
endif()

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${${PROJECT_NAME}_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# Shared library that when placed in ITK_AUTOLOAD_PATH, will add
# DMMLIDImageIO as an ImageIOFactory.  Need to have separate shared
# library for each new format. Note that the plugin library is placed
# in a special directory to speed up the searching for ImageIO
# factories (which improves the speed at which plugins run).

if(NOT DEFINED DMMLIDImageIO_ITKFACTORIES_DIR)
  set(DMMLIDImageIO_ITKFACTORIES_DIR lib/ITKFactories)
endif()
if(NOT DEFINED DMMLIDImageIO_INSTALL_ITKFACTORIES_DIR)
  set(DMMLIDImageIO_INSTALL_ITKFACTORIES_DIR ${DMMLIDImageIO_ITKFACTORIES_DIR})
endif()

add_library(DMMLIDIOPlugin SHARED
  itkDMMLIDIOPlugin.cxx
  )

set_target_properties(DMMLIDIOPlugin PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DMMLIDImageIO_ITKFACTORIES_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DMMLIDImageIO_ITKFACTORIES_DIR}"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DMMLIDImageIO_ITKFACTORIES_DIR}"
  )
target_link_libraries(DMMLIDIOPlugin ${lib_name})

# Folder
if(NOT "${${PROJECT_NAME}_FOLDER}" STREQUAL "")
  set_target_properties(DMMLIDIOPlugin PROPERTIES FOLDER ${${PROJECT_NAME}_FOLDER})
endif()

# --------------------------------------------------------------------------
# Install library - DMMLIDIO and DMMLIDOPlugin are installed in different locations
# --------------------------------------------------------------------------
install(TARGETS DMMLIDIOPlugin
  RUNTIME DESTINATION ${DMMLIDImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  LIBRARY DESTINATION ${DMMLIDImageIO_INSTALL_ITKFACTORIES_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${${PROJECT_NAME}_INSTALL_LIB_DIR} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Set INCLUDE_DIRS variable
# --------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  CACHE INTERNAL "${PROJECT_NAME} include dirs" FORCE)
