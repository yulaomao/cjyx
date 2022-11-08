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
# CjyxMacroBuildModuleWidgets
#

macro(CjyxMacroBuildModuleWidgets)
  set(options
    WRAP_PYTHONQT
    NO_INSTALL
    )
  set(oneValueArgs
    NAME
    EXPORT_DIRECTIVE
    FOLDER
    )
  set(multiValueArgs
    SRCS
    MOC_SRCS
    UI_SRCS
    INCLUDE_DIRECTORIES
    TARGET_LIBRARIES
    RESOURCES
    )
  cmake_parse_arguments(MODULEWIDGETS
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  if(MODULEWIDGETS_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to CjyxMacroBuildModuleWidgets(): \"${MODULEWIDGETS_UNPARSED_ARGUMENTS}\"")
  endif()

  list(APPEND MODULEWIDGETS_INCLUDE_DIRECTORIES
    ${Cjyx_Libs_INCLUDE_DIRS}
    ${Cjyx_Base_INCLUDE_DIRS}
    ${Cjyx_ModuleLogic_INCLUDE_DIRS}
    ${Cjyx_ModuleDMML_INCLUDE_DIRS}
    ${Cjyx_ModuleWidgets_INCLUDE_DIRS}
    )

  list(APPEND MODULEWIDGETS_TARGET_LIBRARIES
    ${Cjyx_GUI_LIBRARY}
    )

  if(NOT DEFINED MODULEWIDGETS_FOLDER AND DEFINED MODULE_NAME)
    set(MODULEWIDGETS_FOLDER "Module-${MODULE_NAME}")
  endif()
  if(NOT "${MODULEWIDGETS_FOLDER}" STREQUAL "")
    set_target_properties(${lib_name} PROPERTIES FOLDER ${MODULEWIDGETS_FOLDER})
  endif()

  set(MODULEWIDGETS_WRAP_PYTHONQT_OPTION)
  if(MODULEWIDGETS_WRAP_PYTHONQT)
    set(MODULEWIDGETS_WRAP_PYTHONQT_OPTION "WRAP_PYTHONQT")
  endif()
  set(MODULEWIDGETS_NO_INSTALL_OPTION)
  if(MODULEWIDGETS_NO_INSTALL)
    set(MODULEWIDGETS_NO_INSTALL_OPTION "NO_INSTALL")
  endif()

  #-----------------------------------------------------------------------------
  # Translation
  #-----------------------------------------------------------------------------
  if(Cjyx_BUILD_I18N_SUPPORT)
    set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Translations/")

    include(CjyxMacroTranslation)
    CjyxMacroTranslation(
      SRCS ${MODULEWIDGETS_SRCS}
      UI_SRCS ${MODULEWIDGETS_UI_SRCS}
      TS_DIR ${TS_DIR}
      TS_BASEFILENAME ${MODULEWIDGETS_NAME}
      TS_LANGUAGES ${Cjyx_LANGUAGES}
      QM_OUTPUT_DIR_VAR QM_OUTPUT_DIR
      QM_OUTPUT_FILES_VAR QM_OUTPUT_FILES
      )
    set_property(GLOBAL APPEND PROPERTY Cjyx_QM_OUTPUT_DIRS ${QM_OUTPUT_DIR})

  else()
    set(QM_OUTPUT_FILES )
  endif()

  # --------------------------------------------------------------------------
  # Build library
  # --------------------------------------------------------------------------
  CjyxMacroBuildModuleQtLibrary(
    NAME ${MODULEWIDGETS_NAME}
    EXPORT_DIRECTIVE ${MODULEWIDGETS_EXPORT_DIRECTIVE}
    FOLDER ${MODULEWIDGETS_FOLDER}
    INCLUDE_DIRECTORIES ${MODULEWIDGETS_INCLUDE_DIRECTORIES}
    SRCS ${MODULEWIDGETS_SRCS} ${QM_OUTPUT_FILES}
    MOC_SRCS ${MODULEWIDGETS_MOC_SRCS}
    UI_SRCS ${MODULEWIDGETS_UI_SRCS}
    TARGET_LIBRARIES ${MODULEWIDGETS_TARGET_LIBRARIES}
    RESOURCES ${MODULEWIDGETS_RESOURCES}
    ${MODULEWIDGETS_WRAP_PYTHONQT_OPTION}
    ${MODULEWIDGETS_NO_INSTALL_OPTION}
    )

  set_property(GLOBAL APPEND PROPERTY CJYX_MODULE_WIDGET_TARGETS ${MODULEWIDGETS_NAME})

  #-----------------------------------------------------------------------------
  # Update Cjyx_ModuleWidgets_INCLUDE_DIRS
  #-----------------------------------------------------------------------------
  set(Cjyx_ModuleWidgets_INCLUDE_DIRS
    ${Cjyx_ModuleWidgets_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    CACHE INTERNAL "Cjyx Module widgets includes" FORCE)

endmacro()

