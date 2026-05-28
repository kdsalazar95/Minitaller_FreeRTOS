#!/usr/bin/env bash
# ============================================================
# setup.sh — Configura el proyecto FreeRTOS Piedra-Papel-Tijeras
# para QEMU ARM Cortex-M3 (MPS2-AN385)
#
# Dependencias (instalar antes):
#   sudo apt install gcc-arm-none-eabi qemu-system-arm cmake git
#   # macOS: brew install --cask gcc-arm-embedded; brew install qemu cmake git
# ============================================================

set -euo pipefail
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

GREEN='\033[0;32m'; CYAN='\033[0;36m'; RESET='\033[0m'
YELLOW='\033[1;33m'
info()  { echo -e "${CYAN}[setup]${RESET} $*"; }
ok()    { echo -e "${GREEN}[  ok ]${RESET} $*"; }
warn()  { echo -e "${YELLOW}[warn]${RESET} $*"; }

# ── 1. Verificar herramientas ────────────────────────────────
info "Verificando herramientas..."
for tool in arm-none-eabi-gcc qemu-system-arm cmake git; do
    if ! command -v "$tool" &>/dev/null; then
        echo "ERROR: '$tool' no encontrado. Instálalo primero."
        exit 1
    fi
done
ok "Todas las herramientas encontradas."

# ── 2. Clonar FreeRTOS-Kernel ────────────────────────────────
if [ ! -d "FreeRTOS-Kernel" ]; then
    info "Clonando FreeRTOS-Kernel (shallow)..."
    git clone --depth 1 \
        https://github.com/FreeRTOS/FreeRTOS-Kernel.git \
        FreeRTOS-Kernel
    ok "FreeRTOS-Kernel clonado."
else
    ok "FreeRTOS-Kernel ya existe, omitiendo clone."
fi

# ── 3. Copiar archivos de soporte del demo MPS2 ──────────────
# El demo oficial de FreeRTOS para MPS2-AN385 con GCC está en:
# https://github.com/FreeRTOS/FreeRTOS/tree/main/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC
DEMO_REPO="FreeRTOS-Demo"
if [ ! -d "$DEMO_REPO" ]; then
    info "Clonando demo MPS2 de FreeRTOS (sparse checkout)..."
    git clone --depth 1 --no-checkout \
        https://github.com/FreeRTOS/FreeRTOS.git \
        "$DEMO_REPO"
    cd "$DEMO_REPO"
    git sparse-checkout set \
        "FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC"
    git checkout
    cd "$DIR"
    ok "Demo MPS2 descargado."
fi

# ── 4. Crear directorio demo_support con archivos necesarios ──
MPS2_DEMO="$DEMO_REPO/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC"
if [ ! -d "demo_support" ]; then
    info "Copiando archivos de soporte MPS2..."
    mkdir -p demo_support
    # Startup y linker script (ruta actual del demo)
    cp "$MPS2_DEMO"/build/gcc/mps2_m3.ld     demo_support/ 2>/dev/null || true
    cp "$MPS2_DEMO"/build/gcc/startup_gcc.c  demo_support/ 2>/dev/null || true
    cp "$MPS2_DEMO"/build/gcc/printf-stdarg.c demo_support/ 2>/dev/null || true
    # Drivers UART / CMSIS
    cp "$MPS2_DEMO"/*.c                      demo_support/ 2>/dev/null || true
    cp "$MPS2_DEMO"/*.h                      demo_support/ 2>/dev/null || true
    # CMSIS headers (si existen)
    cp -r "$MPS2_DEMO"/CMSIS                 demo_support/ 2>/dev/null || true
    ok "Archivos de soporte copiados a demo_support/."
fi

# ── 5. Compilar ──────────────────────────────────────────────
info "Configurando CMake..."
cmake -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev

info "Compilando..."
cmake --build build --parallel

ok "Compilación exitosa: build/freertos_rps.elf"

# ── 6. Construir imagen Docker (si Docker está disponible) ───
if command -v docker &>/dev/null; then
    info "Intentando construir imagen Docker..."
    if docker compose version &>/dev/null; then
        if docker compose build; then
            ok "Imagen Docker construida con docker compose."
        else
            warn "No se pudo construir con docker compose (permiso/sudo)."
            warn "Ejecuta manualmente: sudo docker compose build"
        fi
    else
        if docker build -t freertos-rps .; then
            ok "Imagen Docker construida: freertos-rps"
        else
            warn "No se pudo construir la imagen Docker (permiso/sudo)."
            warn "Ejecuta manualmente: sudo docker build -t freertos-rps ."
        fi
    fi
else
    warn "Docker no está instalado; se omite build de imagen."
fi
echo ""
echo "══════════════════════════════════════════════"
echo "  Para ejecutar en QEMU:"
echo ""
echo "  qemu-system-arm \\"
echo "    -M mps2-an385 \\"
echo "    -cpu cortex-m3 \\"
echo "    -kernel build/freertos_rps.elf \\"
echo "    -serial stdio \\"
echo "    -nographic \\"
echo "    -semihosting \\"
echo "    -semihosting-config enable=on,target=native"
echo ""
echo "  Para salir de QEMU: Ctrl+A, luego X"
echo "══════════════════════════════════════════════"
