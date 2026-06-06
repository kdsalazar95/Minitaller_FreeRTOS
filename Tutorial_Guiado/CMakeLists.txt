cmake_minimum_required( VERSION 3.15 )
project( freertos_taller C )

set( CMAKE_C_STANDARD 99 )

# ── Ruta al kernel (git submodule) ────────────────────────────────────────────
set( FREERTOS_KERNEL_PATH ${CMAKE_SOURCE_DIR}/FreeRTOS-Kernel )

# Verifica que el submodule esté inicializado
if( NOT EXISTS "${FREERTOS_KERNEL_PATH}/tasks.c" )
    message( FATAL_ERROR
        "\n[ERROR] No se encontró el FreeRTOS-Kernel.\n"
        "Ejecuta desde la raíz del repo:\n"
        "  git submodule update --init\n" )
endif()

# ── Fuentes del kernel ────────────────────────────────────────────────────────
set( FREERTOS_SOURCES
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/Posix/port.c
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
)

# ── Includes ──────────────────────────────────────────────────────────────────
set( FREERTOS_INCLUDES
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/Posix
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/Posix/utils
    ${CMAKE_SOURCE_DIR}   # para encontrar FreeRTOSConfig.h
)

# ── Flags comunes ─────────────────────────────────────────────────────────────
set( COMMON_FLAGS -Wall -Wextra -Wno-unused-parameter )

# ── Target: taller (con TODOs) ────────────────────────────────────────────────
add_executable( taller
    src/taller.c
    ${FREERTOS_SOURCES}
)
target_include_directories( taller PRIVATE ${FREERTOS_INCLUDES} )
target_link_libraries( taller pthread )
target_compile_options( taller PRIVATE ${COMMON_FLAGS} )

# ── Target: solucion ──────────────────────────────────────────────────────────
#add_executable( solucion
#    src/taller_solucion.c
#    ${FREERTOS_SOURCES}
#)
#target_include_directories( solucion PRIVATE ${FREERTOS_INCLUDES} )
#target_link_libraries( solucion pthread )
#target_compile_options( solucion PRIVATE ${COMMON_FLAGS} )

# ── Mensaje de ayuda al configurar ───────────────────────────────────────────
message( STATUS "Targets disponibles:" )
message( STATUS "  make taller   → compila tu código (con TODOs)" )
#message( STATUS "  make solucion → compila la solución de referencia" )
