project(qCjyx${MODULE_NAME}SubjectHierarchyPlugins)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_CJYX_${MODULE_NAME_UPPER}_SUBJECT_HIERARCHY_PLUGINS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${DMMLCore_INCLUDE_DIRS}
  ${vtkCjyxSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${qCjyxSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qCjyxSubjectHierarchy${MODULE_NAME}Plugin.cxx
  qCjyxSubjectHierarchy${MODULE_NAME}Plugin.h
  )

set(${KIT}_MOC_SRCS
  qCjyxSubjectHierarchy${MODULE_NAME}Plugin.h
  )

set(${KIT}_UI_SRCS
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

#-----------------------------------------------------------------------------
set(${KIT}_TARGET_LIBRARIES
  vtkCjyxSubjectHierarchyModuleLogic
  qCjyxSubjectHierarchyModuleWidgets
  ${QT_LIBRARIES}
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleQtLibrary(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  FOLDER "Module-${MODULE_NAME}"
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )
