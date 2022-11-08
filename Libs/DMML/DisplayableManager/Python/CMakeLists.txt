

set(displayableManager_PYTHON_SCRIPTS
  dmmlDisplayableManager/vtkScriptedExampleDisplayableManager
  )

ctkMacroCompilePythonScript(
  TARGET_NAME dmmlDisplayableManager
  SCRIPTS "${displayableManager_PYTHON_SCRIPTS}"
  DESTINATION_DIR ${Cjyx_BINARY_DIR}/bin/Python
  INSTALL_DIR ${Cjyx_INSTALL_BIN_DIR}
  )
