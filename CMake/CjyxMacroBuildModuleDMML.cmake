################################################################################
#
#  Program: 3D Cjyx
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# CjyxMacroBuildModuleDMML
#

macro(CjyxMacroBuildModuleDMML)
  set(options
    DISABLE_WRAP_PYTHON
    NO_INSTALL
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE
    FOLDER
    )
  set(multiValueArgs
    SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    )
  cmake_parse_arguments(MODULEDMML
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(MODULEDMML_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to CjyxMacroBuildModuleDMML(): \"${MODULEDMML_UNPARSED_ARGUMENTS}\"")
  endif()

  list(APPEND MODULEDMML_INCLUDE_DIRECTORIES
    ${Cjyx_Libs_INCLUDE_DIRS}
    ${Cjyx_ModuleDMML_INCLUDE_DIRS}
    )

  if(NOT DEFINED MODULEDMML_FOLDER AND DEFINED MODULE_NAME)
    set(MODULEDMML_FOLDER "Module-${MODULE_NAME}")
  endif()

  set(MODULEDMML_NO_INSTALL_OPTION)
  if(MODULEDMML_NO_INSTALL)
    set(MODULEDMML_NO_INSTALL_OPTION "NO_INSTALL")
  endif()

  CjyxMacroBuildModuleVTKLibrary(
    NAME ${MODULEDMML_NAME}
    EXPORT_DIRECTIVE ${MODULEDMML_EXPORT_DIRECTIVE}
    FOLDER ${MODULEDMML_FOLDER}
    SRCS ${MODULEDMML_SRCS}
    INCLUDE_DIRECTORIES ${MODULEDMML_INCLUDE_DIRECTORIES}
    TARGET_LIBRARIES ${MODULEDMML_TARGET_LIBRARIES}
    ${MODULEDMML_NO_INSTALL_OPTION}
    )

  set_property(GLOBAL APPEND PROPERTY CJYX_MODULE_DMML_TARGETS ${MODULEDMML_NAME})

  #-----------------------------------------------------------------------------
  # Update Cjyx_ModuleDMML_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  set(Cjyx_ModuleDMML_INCLUDE_DIRS
    ${Cjyx_ModuleDMML_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "Cjyx Module DMML includes" FORCE)

  # --------------------------------------------------------------------------
  # Python wrapping
  # --------------------------------------------------------------------------
  if(NOT ${MODULEDMML_DISABLE_WRAP_PYTHON} AND VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

    set(Cjyx_Wrapped_LIBRARIES
      )

    CjyxMacroPythonWrapModuleVTKLibrary(
      NAME ${MODULEDMML_NAME}
      SRCS ${MODULEDMML_SRCS}
      WRAPPED_TARGET_LIBRARIES ${Cjyx_Wrapped_LIBRARIES}
      RELATIVE_PYTHON_DIR "."
      )

    # Set python module logic output
    set_target_properties(${MODULEDMML_NAME}Python PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Cjyx_QTLOADABLEMODULES_BIN_DIR}"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Cjyx_QTLOADABLEMODULES_LIB_DIR}"
      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${Cjyx_QTLOADABLEMODULES_LIB_DIR}"
      )

    if(NOT "${MODULEDMML_FOLDER}" STREQUAL "")
      set_target_properties(${MODULEDMML_NAME}Python PROPERTIES FOLDER ${MODULEDMML_FOLDER})
      if(TARGET ${MODULEDMML_NAME}Hierarchy)
        set_target_properties(${MODULEDMML_NAME}Hierarchy PROPERTIES FOLDER ${MODULEDMML_FOLDER})
      endif()
    endif()

    # Export target
    set_property(GLOBAL APPEND PROPERTY Cjyx_TARGETS ${MODULEDMML_NAME}Python)
  endif()

endmacro()
