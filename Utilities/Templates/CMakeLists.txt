install(DIRECTORY Extensions Modules
  DESTINATION ${Cjyx_INSTALL_SHARE_DIR}/Wizard/Templates
  COMPONENT Runtime
  USE_SOURCE_PERMISSIONS
  )

configure_file(
 ${CMAKE_CURRENT_SOURCE_DIR}/Extensions/extension_description.s4ext.in
 ${Cjyx_BINARY_DIR}/Utilities/Templates/Extensions/extension_description.s4ext.in
 COPYONLY
)

include(CjyxExtensionTemplatesGenerator)

# Test LoadableCustomMarkups module.
# TODO: it would be nicer to instantiate the template so that it has a more
# descriptive name than the default "TemplateKey".
add_subdirectory(Modules/LoadableCustomMarkups)
