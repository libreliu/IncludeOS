
#set(LIB_NAME aarch64_${PLATFORM})
# to solve linking failure
set(LIB_NAME aarch64_pc)

set(PLATFORM_OBJECTS


#  start.asm
  start_aarch64.asm
  stop.asm
)

SET(PLATFORM_SOURCES

  platform.cpp
  kernel_start.cpp
  os.cpp
  gic.cpp
  exception_handling.cpp
  init_libc.cpp
  sanity_checks.cpp
  smp.cpp
  #serial1.cpp

  #generic this should be outside platform and arch
  fdt.cpp

  #RPI stuff
  mini_uart.cpp
  bcm2835_gpio.cpp
  bcm2835_rng.cpp
  bcm2836_irq.cpp
  bcm2835_timer.cpp
  bcm2836_local_irq.cpp
)

#add the arch in include path..
include_directories(../../arch/aarch64)
#include_directories(../x86_pc)

enable_language(ASM)
  #project(assembler C ASM)
set_source_files_properties(${PLATFORM_OBJECTS} PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
  #add_executable(hello foo.s bar.c)

#  set_source_files_properties(start.asm PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")

add_library(${LIB_NAME} STATIC ${PLATFORM_OBJECTS} ${PLATFORM_SOURCES})
#set_source_files_properties(${PLATFORM_SOURCES} PROPERIES LINKER_LANGUAGE CXX)
set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${LIB_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/platform
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/platform
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/platform
)

install(TARGETS ${LIB_NAME} DESTINATION platform)
