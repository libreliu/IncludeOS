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
  )


add_library(hw OBJECT ${SRCS})
