set(SRCS
    block.cpp
    cpuid.cpp
    elf.cpp
    events.cpp
    fiber.cpp
    memmap.cpp
    multiboot.cpp
    os.cpp
    profile.cpp
    syscalls.cpp
    service_stub.cpp
    terminal.cpp
    timers.cpp
    rng.cpp
    tls.cpp
    vga.cpp
    context.cpp
    )
if (NOT CMAKE_TESTING_ENABLED)
  list(APPEND SRCS
  #  rdrand.cpp
    heap.cpp
    kernel.cpp
    liveupdate.cpp
    rtc.cpp
    system_log.cpp
  )
endif()

add_library(kernel OBJECT ${SRCS})
