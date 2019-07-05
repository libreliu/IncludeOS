#
# Build and install plugins as libraries
#

# make LiveUpdate visible to plugins

set(PLUGINS "")
foreach(PLUGIN ${PLUGIN_SRCS})
  get_filename_component(PLUGIN_NAME ${PLUGIN} NAME_WE)
  add_library(${PLUGIN_NAME} STATIC ${PLUGIN})
  list(APPEND PLUGINS ${PLUGIN_NAME})
  install(TARGETS ${DRIVER_NAME} DESTINATION drivers)
endforeach()

add_library(field_medic STATIC
            field_medic/fieldmedic.cpp
            field_medic/diag.cpp)
list(APPEND PLUGINS field_medic)

#handle build targets for conan editable
set_target_properties(${PLUGINS}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/plugins
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/plugins
)

#handle install of all plugins
install(TARGETS ${PLUGINS} DESTINATION plugins)
