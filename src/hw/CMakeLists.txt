set(SRCS
    pci_device.cpp
    pci_manager.cpp
    nic.cpp
    ps2.cpp
    serial.cpp
    vga_gfx.cpp
    msi.cpp
    pci_msi.cpp
    usernet.cpp
    gpio.cpp
    screen.cpp
  )
add_custom_command(
  OUTPUT binary_font_psf.o
  COMMAND aarch64-linux-gnu-objcopy -I binary -O elf64-littleaarch64 -B aarch64 ${CMAKE_CURRENT_SOURCE_DIR}/font.psf binary_font_psf.o
  DEPENDS font.psf
)

add_library(hw_font STATIC binary_font_psf.o)
set_target_properties(hw_font PROPERTIES LINKER_LANGUAGE CXX)

add_library(hw OBJECT ${SRCS})
