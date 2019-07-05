# Add drivers as libraries
#
# Note: we can't have all the drivers in the same library as we'll want
# e.g. several network drivers, which will result in conflicting symbols
#
# ...There are probably nicer solutions, so please PR if you know them.

# make LiveUpdate visible to drivers systemlog.. yet another DEP!!
include_directories(${INCLUDEOS_ROOT}/lib/LiveUpdate)

set(DRIVER_SRCS
  stdout/timestamps.cpp
)

set(DRIVERS "")
foreach(DRIVER ${DRIVER_SRCS})
  get_filename_component(DRIVER_NAME ${DRIVER} NAME_WE)
  add_library(${DRIVER_NAME} STATIC ${DRIVER})
  list(APPEND DRIVERS ${DRIVER_NAME})
endforeach()

#TODO rename source file
add_library(boot_logger STATIC stdout/bootlog.cpp)
list(APPEND DRIVERS
  boot_logger
)
set_target_properties(${DRIVERS}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/drivers
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/drivers
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/drivers
)
install(TARGETS ${DRIVERS} DESTINATION drivers)
# Simple stuff
add_library(default_stdout STATIC stdout/default_stdout.cpp)

set_target_properties(default_stdout
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/drivers/stdout
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/drivers/stdout
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/drivers/stdout
)

#
# Installation
#
set(CMAKE_INSTALL_MESSAGE LAZY) # to avoid spam

# installation of stdout drivers
install(TARGETS
    default_stdout
  DESTINATION drivers/stdout)
