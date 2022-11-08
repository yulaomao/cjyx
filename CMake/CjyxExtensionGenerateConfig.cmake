# Generate the <Extension>Config.cmake file in the build tree.
# This file makes it possible for an extension to depend on targets defined
# in other extensions.
# This file is based on CjyxGenerateCjyxConfig.cmake.
#
# Example:
#
# Add these lines near the end of top-level CMakeLists.txt in Sequences extension
# to allow other extensions to use it.
#
#   # Pass some custom content to the SequencesConfig.cmake file (optional)
#   set(Sequences_CUSTOM_CONFIG "message(STATUS \"Test\")")
#
#   # Generate SequencesConfig.cmake
#   include(${Cjyx_EXTENSION_GENERATE_CONFIG})
#
# Add these lines to the CMakeLists.txt to use vtkDMMLSequenceNode
# implemented in Sequences extension:
#
#   find_package(Sequences REQUIRED)
#   ...
#   set(MODULE_INCLUDE_DIRECTORIES
#     ...
#     ${vtkCjyxSequencesModuleDMML_INCLUDE_DIRS}
#     )
#   ...
#   set(MODULE_TARGET_LIBRARIES
#     ...
#     vtkCjyxSequencesModuleDMML
#     )
#

if(NOT DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
  # This extension is built as part of the main application.
  # Targets and paths are managed the same way as for core modules.
  return()
endif()

get_property(_module_targets GLOBAL PROPERTY CJYX_MODULE_TARGETS)
if(_module_targets)
  foreach(target ${_module_targets})
    set(EXTENSION_INCLUDE_MODULE_DIRS_CONFIG
"${EXTENSION_INCLUDE_MODULE_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
  endforeach()
endif()

get_property(_module_logic_targets GLOBAL PROPERTY CJYX_MODULE_LOGIC_TARGETS)
if(_module_logic_targets)
  foreach(target ${_module_logic_targets})
    set(EXTENSION_INCLUDE_MODULE_LOGIC_DIRS_CONFIG
"${EXTENSION_INCLUDE_MODULE_LOGIC_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
    list(APPEND EXTENSION_ModuleLogic_INCLUDE_DIRS_CONFIG "\${${target}_INCLUDE_DIRS}")
  endforeach()
endif()

get_property(_module_dmml_targets GLOBAL PROPERTY CJYX_MODULE_DMML_TARGETS)
if(_module_dmml_targets)
  foreach(target ${_module_dmml_targets})
    set(EXTENSION_INCLUDE_MODULE_DMML_DIRS_CONFIG
"${EXTENSION_INCLUDE_MODULE_DMML_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
    list(APPEND EXTENSION_ModuleDMML_INCLUDE_DIRS_CONFIG "\${${target}_INCLUDE_DIRS}")
  endforeach()
endif()

get_property(_module_widget_targets GLOBAL PROPERTY CJYX_MODULE_WIDGET_TARGETS)
if(_module_widget_targets)
  foreach(target ${_module_widget_targets})
    set(EXTENSION_INCLUDE_MODULE_WIDGET_DIRS_CONFIG
"${EXTENSION_INCLUDE_MODULE_WIDGET_DIRS_CONFIG}
set(${target}_INCLUDE_DIRS
  \"${${target}_INCLUDE_DIRS}\")"
)
    list(APPEND EXTENSION_ModuleWidgets_INCLUDE_DIRS_CONFIG "\${${target}_INCLUDE_DIRS}")
  endforeach()
endif()

get_property(_wrap_hierarchy_targets GLOBAL PROPERTY CJYX_WRAP_HIERARCHY_TARGETS)
if(_wrap_hierarchy_targets)
  foreach(target ${_wrap_hierarchy_targets})
    set(EXTENSION_WRAP_HIERARCHY_FILES_CONFIG
"${EXTENSION_WRAP_HIERARCHY_FILES_CONFIG}
set(${target}_WRAP_HIERARCHY_FILE
  \"${${target}_WRAP_HIERARCHY_FILE}\")"
)
  endforeach()
endif()

set(EXTENSION_SOURCE_DIR_CONFIG "set(${EXTENSION_NAME}_SOURCE_DIR \"${${EXTENSION_NAME}_SOURCE_DIR}\")")

# Variables that will be used for populating AdditionalLauncherSettings.ini.
set(EXTENSION_ADDITIONAL_LAUNCHER_SETTINGS_FILE_CONTENT_CONFIG "set(${EXTENSION_NAME}_LIBRARY_PATHS_LAUNCHER_BUILD \"${EXTENSION_LIBRARY_PATHS_BUILD}\")
set(${EXTENSION_NAME}_PATHS_LAUNCHER_BUILD \"${EXTENSION_PATHS_BUILD}\")
set(${EXTENSION_NAME}_ENVVARS_LAUNCHER_BUILD \"${EXTENSION_LAUNCHER_SETTINGS_ENVVARS}\")
set(${EXTENSION_NAME}_PYTHONPATH_LAUNCHER_BUILD \"${EXTENSION_PYTHONPATH_BUILD}\")")

# Allow extensions to add some custom content.
if(${EXTENSION_NAME}_CUSTOM_CONFIG)
  set(EXTENSION_CUSTOM_CONFIG "${${EXTENSION_NAME}_CUSTOM_CONFIG}")
else()
  set(EXTENSION_CUSTOM_CONFIG "")
endif()

# Export Targets file.
set(EXTENSION_TARGETS_FILE "${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY}/${EXTENSION_NAME}Targets.cmake")
get_property(Cjyx_TARGETS GLOBAL PROPERTY Cjyx_TARGETS)
export(TARGETS ${Cjyx_TARGETS} FILE ${EXTENSION_TARGETS_FILE})

# Configure <Extension>Config.cmake for the build tree.
configure_file(
  ${Cjyx_CMAKE_DIR}/CjyxExtensionConfig.cmake.in
  ${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY}/${EXTENSION_NAME}Config.cmake @ONLY)
