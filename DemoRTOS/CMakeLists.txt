cmake_minimum_required(VERSION 3.16)

# ── Toolchain cruzado: ARM GCC ──────────────────────────────
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

find_program(ARM_GCC arm-none-eabi-gcc REQUIRED)
find_program(ARM_GPP arm-none-eabi-g++ REQUIRED)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)

set(CMAKE_C_COMPILER   ${ARM_GCC})
set(CMAKE_CXX_COMPILER ${ARM_GPP})
set(CMAKE_ASM_COMPILER ${ARM_GCC})

project(freertos_rps C ASM)

# ── Flags de compilacion ────────────────────────────────────
set(CPU_FLAGS
    "-mcpu=cortex-m3"
    "-mthumb"
    "-mfloat-abi=soft"
)
string(JOIN " " CPU_FLAGS_STR ${CPU_FLAGS})

set(CMAKE_C_FLAGS "${CPU_FLAGS_STR} -Wall -Wextra -O2 -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS
    "${CPU_FLAGS_STR} -Wl,--gc-sections -Wl,-Map=output.map -nostartfiles"
)

# ── Ruta al FreeRTOS-Kernel (clonado por setup.sh) ──────────
set(FREERTOS_DIR "${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel")

# ── Puerto ARM CM3 GCC ──────────────────────────────────────
set(PORT_DIR "${FREERTOS_DIR}/portable/GCC/ARM_CM3")
set(HEAP_DIR "${FREERTOS_DIR}/portable/MemMang")

# ── Fuentes del kernel ──────────────────────────────────────
file(GLOB KERNEL_SOURCES
    "${FREERTOS_DIR}/tasks.c"
    "${FREERTOS_DIR}/queue.c"
    "${FREERTOS_DIR}/list.c"
    "${FREERTOS_DIR}/timers.c"
    "${FREERTOS_DIR}/event_groups.c"
    "${FREERTOS_DIR}/stream_buffer.c"
    "${PORT_DIR}/port.c"
    "${HEAP_DIR}/heap_4.c"
)

# ── Fuentes de la demo MPS2 (startup + UART) ────────────────
set(DEMO_DIR "${CMAKE_SOURCE_DIR}/demo_support")
set(DEMO_ORIG_DIR "${CMAKE_SOURCE_DIR}/FreeRTOS-Demo/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC")
set(DEMO_GCC_DIR "${DEMO_ORIG_DIR}/build/gcc")

set(DEMO_SOURCES
    "${DEMO_GCC_DIR}/startup_gcc.c"
    "${DEMO_GCC_DIR}/printf-stdarg.c"
)

# ── Ejecutable final ─────────────────────────────────────────
add_executable(freertos_rps.elf
    main.c
    syscalls.c
    ${KERNEL_SOURCES}
    ${DEMO_SOURCES}
)

target_include_directories(freertos_rps.elf PRIVATE
    ${CMAKE_SOURCE_DIR}           # FreeRTOSConfig.h
    ${FREERTOS_DIR}/include
    ${PORT_DIR}
    ${DEMO_DIR}
    ${DEMO_ORIG_DIR}/CMSIS
)

# Linker script del demo MPS2
target_link_options(freertos_rps.elf PRIVATE
    "-T${DEMO_GCC_DIR}/mps2_m3.ld"
)

# ── Generar binario .bin (opcional para QEMU raw) ────────────
add_custom_command(TARGET freertos_rps.elf POST_BUILD
    COMMAND ${ARM_OBJCOPY} -O binary freertos_rps.elf freertos_rps.bin
    COMMENT "Generando freertos_rps.bin"
)
